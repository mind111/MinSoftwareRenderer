#include "../Include/Shader.h"

void VertexShader::ProcessTriangle(Vec3<float> V0,
                                   Vec3<float> V1,
                                   Vec3<float> V2,
                                   Vec2<float>* Out)
{
        Vec4<float> V0_Augmented(V0.x, V0.y, V0.z, 1.0f);
        Vec4<float> V1_Augmented(V1.x, V1.y, V1.z, 1.0f);
        Vec4<float> V2_Augmented(V2.x, V2.y, V2.z, 1.0f);

        // Model->World->Camera->Clip
        Vec4<float> V0Clip_Vec4 = MVP * V0_Augmented;
        Vec4<float> V1Clip_Vec4 = MVP * V1_Augmented;
        Vec4<float> V2Clip_Vec4 = MVP * V2_Augmented;
        
        // Need to divde (w-component - 1) of transformed points
        V0Clip_Vec4.x = V0Clip_Vec4.x / (V0Clip_Vec4.w - 1);
        V0Clip_Vec4.y = V0Clip_Vec4.y / (V0Clip_Vec4.w - 1);
        V1Clip_Vec4.x = V1Clip_Vec4.x / (V1Clip_Vec4.w - 1);
        V1Clip_Vec4.y = V1Clip_Vec4.y / (V1Clip_Vec4.w - 1);
        V2Clip_Vec4.x = V2Clip_Vec4.x / (V2Clip_Vec4.w - 1);
        V2Clip_Vec4.y = V2Clip_Vec4.y / (V2Clip_Vec4.w - 1);
        
        // Resetting the w back to 1 or else would mess up the computation
        // for Viewport transformation
        V0Clip_Vec4.w = 1.f;
        V1Clip_Vec4.w = 1.f;
        V2Clip_Vec4.w = 1.f;

        // Then apply viewport transformation
        Vec4<float> V0Screen_Vec4 = this->Viewport * V0Clip_Vec4;
        Vec4<float> V1Screen_Vec4 = this->Viewport * V1Clip_Vec4;
        Vec4<float> V2Screen_Vec4 = this->Viewport * V2Clip_Vec4;
        
        Vec2<float> V0Screen(V0Screen_Vec4.x, V0Screen_Vec4.y);
        Vec2<float> V1Screen(V1Screen_Vec4.x, V1Screen_Vec4.y);
        Vec2<float> V2Screen(V2Screen_Vec4.x, V2Screen_Vec4.y);
        
        *(Out) = V0Screen;
        *(Out + 1) = V1Screen;
        *(Out + 2) = V2Screen;
}
/** 
 * Given a point P on triangle's projection on screen space, use barycentric
 * coordinates to derive/interpolate P's actual position in world space.
 * 2D -> 3D. 
 * ** Orthographic case **
 * projection still preserves same barycentric coordinates as projected triangle
 * in 3D space
 * ** Perspective case **
 * TODO:
 */
bool FragmentShader::UpdateDepthBuffer(Vec3<float> V0, Vec3<float> V1, Vec3<float> V2, 
                       int ScreenX, int ScreenY, Vec3<float> Weights, float* ZBuffer)
{
    int Index = ScreenY * 800 + ScreenX;
    float z = V0.z * Weights.x + V1.z * Weights.y + V2.z * Weights.z;

    if (z > ZBuffer[Index]) 
    {
        ZBuffer[Index] = z;
        return true;
    }

    return false;
}

void FragmentShader::RasterizeTriangle(Vec2<float> V0Screen, Vec2<float> V1Screen, 
        Vec2<float> V2Screen, 
        Vec3<float> V0_World,
        Vec3<float> V1_World,
        Vec3<float> V2_World,
        TGAImage& image, 
        Vec2<float>& V0_UV, 
        Vec2<float>& V1_UV, 
        Vec2<float>& V2_UV, 
        TGAImage* TextureImage,
        float* ZBuffer)
{
    Vec2<float> E1 = V1Screen - V0Screen;
    Vec2<float> E2 = V2Screen - V0Screen;

    // Ignore triangles whose three vertices lie in the same line
    // in screen space
    // (V1.x - V0.x) * (V2.y - V0.y) == (V2.x - V0.x) * (V1.y - V0.y)
    if (E1.x * E2.y == E2.x * E1.y) return;

    // Calculate the bounding box for the triangle
    int Bottom = V0Screen.y, Up = V0Screen.y, Left = V0Screen.x, Right = V0Screen.x;
    float Denom = E1.x * E2.y - E2.x * E1.y;

    Vec2<float> T[3];
    T[0] = V0Screen;
    T[1] = V1Screen;
    T[2] = V2Screen;
    
    for (int i = 0; i < 3; i++)
    {
        if (T[i].x < Left) Left = T[i].x;
        if (T[i].x > Right) Right = T[i].x;
        if (T[i].y < Bottom) Bottom = T[i].y;
        if (T[i].y > Up) Up = T[i].y;
    }

    if (Left > 799.f)   Left   = 799.f;
    if (Right > 799.f)  Right  = 799.f;
    if (Up > 799.f)     Up     = 799.f;
    if (Bottom > 799.f) Bottom = 799.f;

    ///\TODO: Need to further refine rounding issue when deciding if a pixel
    //        overlaps a triangle
    // Rasterization
    for (int x = Left; x <= (int)Right; x++)
    {
        for (int y = Bottom; y <= (int)Up; y++)
        {
            /// \Note: Cramer's rule to solve for barycentric coordinates,
            ///       can also use ratio of area between three sub-triangles to solve
            ///       to solve for u,v,w
            Vec2<float> PA = V0Screen - Vec2<float>(x + .5f, y + .5f);

            float u = (-1 * PA.x * E2.y + PA.y * E2.x) / Denom;
            float v = (-1 * PA.y * E1.x + PA.x * E1.y) / Denom;
            float w = 1 - u - v;

            Vec3<float> Weights(u, v, w);

            // Point p is not inside of the triangle
            if (u < 0.f || v < 0.f || w < 0.f)
                continue;

            // Depth test to see if current pixel is visible 
            if (UpdateDepthBuffer(V0_World, V1_World, V2_World, x, y, Weights, ZBuffer))
            {
                Vec2<float> MappedTexturePos(
                        Weights.z * V0_UV.x + Weights.x * V1_UV.x + Weights.y * V2_UV.x,
                        Weights.z * V0_UV.y + Weights.x * V1_UV.y + Weights.y * V2_UV.y
                );

                TGAColor Color = TextureImage->get(
                        TextureImage->get_width() * MappedTexturePos.x, 
                        TextureImage->get_height() * MappedTexturePos.y
                );

                image.set(x, y, TGAColor(
                            Color.bgra[2], 
                            Color.bgra[1], 
                            Color.bgra[0], 
                            255
                ));
            }
        }
    }

    return;
}

