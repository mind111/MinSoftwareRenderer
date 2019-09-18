#include "../Include/Shader.h"

#define Ka 0.1f // Ambient coef
#define Kd 0.4f // Diffuse coef
#define Ks 0.3f // Specular coef

void VertexShader::Vertex_Shader(Vec3<float> V0,
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
   
    // Arrange the vertices based on their y-coordinates
    *(Out) = V0Screen;
    *(Out + 1) = V1Screen;
    *(Out + 2) = V2Screen;
}

void FragmentShader::Gouraud_Shader(Vec2<float> Fragment, 
                                    float Diffuse_Coef, 
                                    TGAImage& image,
                                    TGAColor Color)
{
    image.set(Fragment.x, Fragment.y, TGAColor(229 * Diffuse_Coef, 200 * Diffuse_Coef, 232 * Diffuse_Coef));
}

void FragmentShader::Toon_Shader(Vec2<float> Fragment, 
                                 float Diffuse_Coef, 
                                 TGAImage& image,
                                 TGAColor Color)
{
    float Toon_Threshold[5] = {
        0.1f,
        0.3f,
        0.5f,
        0.7f,
        0.9f
    };

    // Toon post-processing
    if (Diffuse_Coef < Toon_Threshold[0]) Diffuse_Coef = 0.05f;
    else if (Diffuse_Coef < Toon_Threshold[1]) Diffuse_Coef = 0.25f;
    else if (Diffuse_Coef < Toon_Threshold[2]) Diffuse_Coef = 0.45f;
    else if (Diffuse_Coef < Toon_Threshold[3]) Diffuse_Coef = 0.65f;
    else if (Diffuse_Coef < Toon_Threshold[4]) Diffuse_Coef = 0.85f;
    else Diffuse_Coef = 1.f;

    image.set(Fragment.x, Fragment.y, TGAColor(Color[0] * Diffuse_Coef, Color[1] * Diffuse_Coef, Color[2] * Diffuse_Coef));
}

float Toon_PostProcess(float* Toon_Value, int ValueCount, float Value)
{
    for (int i = 0; i < ValueCount; i++)
    {
        if (Value < Toon_Value[i])
            return Toon_Value[i] - .05f;
    }
    return 1.f;
}

// @Param: Color defines light color for now
void FragmentShader::Phong_Shader(Vec2<int> Fragment, 
                                  Vec3<float> n,
                                  Vec3<float> LightDir,
                                  Vec3<float> ViewDir,
                                  TGAImage& image,
                                  TGAColor Color)
{
    TGAColor Material(229, 200, 232); // Temporary place holder for object's color

    float Toon_Threshold[5] = {
        0.1f,
        0.3f,
        0.5f,
        0.7f,
        0.9f
    };

    // Compute reflection light
    Vec3<float> ReflLightDir = n * 2.f * MathFunctionLibrary::DotProduct_Vec3(n, LightDir) - LightDir;
    float Diffuse_Coef = MathFunctionLibrary::DotProduct_Vec3(n, LightDir);
    float Specular_Coef = MathFunctionLibrary::DotProduct_Vec3(ViewDir, ReflLightDir);

    TGAColor Phong_Color;
    for (int i = 0; i < 3; i++) 
        Phong_Color[i] = std::min<float>(Ka * Material[i] + Color[i] * Kd * Diffuse_Coef + Color[i] * Ks * std::pow(Specular_Coef, 10), 255); 

    image.set(Fragment.x, Fragment.y, Phong_Color);
}

void FragmentShader::Fragment_Shader(Vec2<int> Fragment, 
                                     Vec2<float> V0_UV,
                                     Vec2<float> V1_UV, 
                                     Vec2<float> V2_UV,
                                     Vec3<float>& Weights,
                                     TGAImage* TextureAsset,
                                     TGAImage& image)
{
                TGAColor Color = this->SampleTexture(TextureAsset, Weights, V0_UV, V1_UV, V2_UV);
                image.set(Fragment.x, Fragment.y, Color);
}

TGAColor FragmentShader::SampleTexture(TGAImage* TextureImage, 
                                       Vec3<float> Weights, 
                                       Vec2<float> V0_UV, 
                                       Vec2<float> V1_UV, 
                                       Vec2<float> V2_UV)
{
    Vec2<float> MappedTexturePos(
            Weights.z * V0_UV.x + Weights.x * V1_UV.x + Weights.y * V2_UV.x,
            Weights.z * V0_UV.y + Weights.x * V1_UV.y + Weights.y * V2_UV.y
    );

    TGAColor Color = TextureImage->get(
            TextureImage->get_width() * MappedTexturePos.x, 
            TextureImage->get_height() * MappedTexturePos.y
    );

    return Color;
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
bool FragmentShader::UpdateDepthBuffer(Vec3<float> V0, 
                                       Vec3<float> V1, 
                                       Vec3<float> V2, 
                                       int ScreenX, 
                                       int ScreenY, 
                                       Vec3<float> Weights)
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


void Shader::Draw(Model& Model, TGAImage& image, Camera& Camera, Shader_Mode ShadingMode)
{
    Vec3<float> LightDir(0.f, 0.f, 1.f);
    LightDir = MathFunctionLibrary::Normalize(LightDir);

    Vec3<int>* IndexPtr = Model.Indices;
    int TriangleRendered = 0;
    float Diffuse_Coefs[3];

    while (TriangleRendered < 2492) //TODO: This hard-coded value should be replaced
    {
        Vec3<float> V0V1 =  Model.VertexBuffer[(IndexPtr + 1)->x] - Model.VertexBuffer[IndexPtr->x];
        Vec3<float> V0V2 =  Model.VertexBuffer[(IndexPtr + 2)->x] - Model.VertexBuffer[IndexPtr->x];

        // Calculate diffuse coef at each vertex here
        Diffuse_Coefs[0] = MathFunctionLibrary::DotProduct_Vec3(
                            Model.VertexNormalBuffer[IndexPtr->z], LightDir 
                );

        Diffuse_Coefs[1] = MathFunctionLibrary::DotProduct_Vec3(
                            Model.VertexNormalBuffer[(IndexPtr + 1)->z], LightDir
                );
        
        Diffuse_Coefs[2] = MathFunctionLibrary::DotProduct_Vec3(
                            Model.VertexNormalBuffer[(IndexPtr + 2)->z], LightDir 
                );

        // Derive surface normal, counter-clock wise winding order
        Vec3<float> Surface_Normal = MathFunctionLibrary::Normalize(
                MathFunctionLibrary::CrossProduct(V0V1, V0V2));

        float ShadingCoef = MathFunctionLibrary::DotProduct_Vec3(Vec3<float>(0, 0, 1), Surface_Normal);
        // ShadingCoef < 0 means that the triangle is facing away from the light, simply discard
        if (ShadingCoef < 0.0f) 
        {
            TriangleRendered++;
            IndexPtr += 3;
            continue;
        }
        
        if (ShadingCoef > 1.0f) ShadingCoef = 1.0f;

        VS.Vertex_Shader(Model.VertexBuffer[IndexPtr->x],      
                         Model.VertexBuffer[(IndexPtr + 1)->x],
                         Model.VertexBuffer[(IndexPtr + 2)->x],
                         this->Triangle);

        Vec2<float> E1 = Triangle[1] - Triangle[0];
        Vec2<float> E2 = Triangle[2] - Triangle[0];

        // Ignore triangles whose three vertices lie in the same line
        // in screen space
        // (V1.x - V0.x) * (V2.y - V0.y) == (V2.x - V0.x) * (V1.y - V0.y)
        float Denom = E1.x * E2.y - E2.x * E1.y;
        if (Denom == 0) return;
        
        // Calculate the bounding box for the triangle
        int Bottom = Triangle[0].y, Up = Triangle[0].y, Left = Triangle[0].x, Right = Triangle[0].x;

        for (int i = 0; i < 3; i++)
        {
            if (Triangle[i].x < Left) Left = Triangle[i].x;
            if (Triangle[i].x > Right) Right = Triangle[i].x;
            if (Triangle[i].y < Bottom) Bottom = Triangle[i].y;
            if (Triangle[i].y > Up) Up = Triangle[i].y;
        }

        if (Left > 799.f)   Left   = 799.f;
        if (Right > 799.f)  Right  = 799.f;
        if (Up > 799.f)     Up     = 799.f;
        if (Bottom > 799.f) Bottom = 799.f;

        for (int x = Left; x <= (int)Right; x++)
        {
            for (int y = Bottom; y <= (int)Up; y++)
            {
                /// \Note: Cramer's rule to solve for barycentric coordinates,
                ///       can also use ratio of area between three sub-triangles to solve
                ///       to solve for u,v,w
                Vec2<float> PA = Triangle[0] - Vec2<float>(x + .5f, y + .5f);

                float u = (-1 * PA.x * E2.y + PA.y * E2.x) / Denom;
                float v = (-1 * PA.y * E1.x + PA.x * E1.y) / Denom;
                float w = 1 - u - v;

                Vec3<float> Weights(u, v, w);

                // Point p is not inside of the triangle
                if (u < 0.f || v < 0.f || w < 0.f)
                    continue;

                // Depth test to see if current pixel is visible 
                if (this->FS.UpdateDepthBuffer(Model.VertexBuffer[IndexPtr->x],      
                                      Model.VertexBuffer[(IndexPtr + 1)->x],
                                      Model.VertexBuffer[(IndexPtr + 2)->x],
                                      x, y, Weights))
                {
                    switch (ShadingMode)
                    {
                        case Shader_Mode::Flat_Shader:
                        {
                            Vec2<float> V0_UV = Model.TextureBuffer[IndexPtr->y];
                            Vec2<float> V1_UV = Model.TextureBuffer[(IndexPtr + 1)->y];
                            Vec2<float> V2_UV = Model.TextureBuffer[(IndexPtr + 2)->y];

                            FS.Fragment_Shader(Vec2<int>(x, y),
                                               V0_UV, 
                                               V1_UV,
                                               V2_UV,
                                               Weights,
                                               Model.TextureAssets[0], 
                                               image);
                        }

                        case Shader_Mode::Gouraud_Shader:
                        {
                            float Diffuse_Coef = Weights.z * Diffuse_Coefs[0] + Weights.x * Diffuse_Coefs[1] + Weights.y * Diffuse_Coefs[2];
                            if (Diffuse_Coef > 1.f) Diffuse_Coef = 1.f;
                            if (Diffuse_Coef < 0.f) Diffuse_Coef = 0.f;

                            FS.Gouraud_Shader(Vec2<float>(x, y), Diffuse_Coef, image,
                                    TGAColor(255, 255, 255));

                            break;
                        }

                        case Shader_Mode::Phong_Shader:
                        {
                            // Interpolate normal and feed to fragment shader
                            Vec3<float> N0 = Model.VertexNormalBuffer[IndexPtr->z];
                            Vec3<float> N1 = Model.VertexNormalBuffer[(IndexPtr + 1)->z];
                            Vec3<float> N2 = Model.VertexNormalBuffer[(IndexPtr + 2)->z];

                            Vec3<float> n = MathFunctionLibrary::Normalize(N0 * Weights.z + N1 * Weights.x + N2 * Weights.y);

                            // TODO: Need to swap out the hard-coded viewing direction later
                            FS.Phong_Shader(Vec2<int>(x, y), n, LightDir, Vec3<float>(1.f, .5f, 1.f), image, TGAColor(255, 255, 255));

                            break;
                        }
 
                        case Shader_Mode::Toon_Shader:
                        {
                            break;
                        }

                        default:
                            break;
                    }
                }
            }
        }

        IndexPtr += 3;    
        TriangleRendered++;
    }
}
