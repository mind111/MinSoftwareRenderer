#include "../include/Shader.h"
#include "../include/scene.h"

#define Ka 0.5f // Ambient coef
#define Kd 0.4f // Diffuse coef
#define Ks 0.1f // Specular coef

/// \TODO: Simplify logic and optimization
void DrawLine(Vec2<int> Start, Vec2<int> End, TGAImage& image, const TGAColor& color)
{
    int d = 1;
    //**** Divide by 0
    if (Start.x == End.x)
    {
        if (Start.y > End.y) Start.Swap(End);
        for (int i = Start.y; i <= End.y; i++)
        {
            image.set(Start.x, i, color);
        }

        return;
    }
    // Always start from left marching toward right
    // Swap Start and End
    if (Start.x > End.x) Start.Swap(End);
    // Derive the line function
    float Slope = (float)(Start.y - End.y) / (float)(Start.x - End.x);
    float Intercept = (float)End.y - Slope * End.x;
    Vec2<int> Next(Start);
    Vec2<int> StepA(1, 0);
    Vec2<int> StepB(1, 1);

    if (Slope < 0)
    {
        d *= -1;
        StepA.Swap(StepB);
    }
    if (Slope > 1 || Slope < -1)
    {
        StepA.Transpose();
        StepB.Transpose();
    }

    StepA.y *= d;
    StepB.y *= d;

    while (Next.x < End.x || Next.y != End.y)
    {
        //**** Slope < 1
        if (Slope >= -1 && Slope <= 1)
        {
            // Eval F(x+1,y+0.5)
            if (Next.y + d * 0.5f - Slope * (Next.x + 1.0f) - Intercept >= 0)
                Next += StepA;
            else
                Next += StepB;
        }
        /// \Bug: When slope is greater than 0, can invert x, y
        else
        {
            // Eval F(x+0.5, y+1)
            if (Next.y + d * 1.0f - Slope * (Next.x + .5f) - Intercept >= 0)
                Next += StepB;
            else
                Next += StepA;
        }
        //**** Draw pixel to the buffer
        image.set(Next.x, Next.y, color);
    }
}

void DrawTriangle(Vec2<int> V0, Vec2<int> V1, Vec2<int> V2, TGAImage& image, const TGAColor& color)
{
    DrawLine(V0, V1, image, color);
    DrawLine(V1, V2, image, color);
    DrawLine(V2, V0, image, color);
}

void VertexShader::Vertex_Shader(Vec3<float> V0,
                                 Vec3<float> V1,
                                 Vec3<float> V2,
                                 Vec2<float>* Out,
                                 Vec3<float>* Out_Clip)
{
    Vec4<float> V0_Augmented(V0, 1.0f);
    Vec4<float> V1_Augmented(V1, 1.0f);
    Vec4<float> V2_Augmented(V2, 1.0f);

    // Model->World->Camera->Clip
    Vec4<float> V0Clip_Vec4 = MVP * V0_Augmented;
    Vec4<float> V1Clip_Vec4 = MVP * V1_Augmented;
    Vec4<float> V2Clip_Vec4 = MVP * V2_Augmented;
    
    V0Clip_Vec4.x = V0Clip_Vec4.x / V0Clip_Vec4.w;
    V0Clip_Vec4.y = V0Clip_Vec4.y / V0Clip_Vec4.w;
    V0Clip_Vec4.z = V0Clip_Vec4.z / V0Clip_Vec4.w;
    V1Clip_Vec4.x = V1Clip_Vec4.x / V1Clip_Vec4.w;
    V1Clip_Vec4.y = V1Clip_Vec4.y / V1Clip_Vec4.w;
    V1Clip_Vec4.z = V1Clip_Vec4.z / V1Clip_Vec4.w;
    V2Clip_Vec4.x = V2Clip_Vec4.x / V2Clip_Vec4.w;
    V2Clip_Vec4.y = V2Clip_Vec4.y / V2Clip_Vec4.w;
    V2Clip_Vec4.z = V2Clip_Vec4.z / V2Clip_Vec4.w;
    
    // Resetting the w back to 1 or else would mess up the computation
    // for Viewport transformation
    V0Clip_Vec4.w = 1.f;
    V1Clip_Vec4.w = 1.f;
    V2Clip_Vec4.w = 1.f;

    // Then apply viewport transformation
    Vec4<float> V0Screen_Vec4 = this->Viewport * V0Clip_Vec4;
    Vec4<float> V1Screen_Vec4 = this->Viewport * V1Clip_Vec4;
    Vec4<float> V2Screen_Vec4 = this->Viewport * V2Clip_Vec4;
    
    // TODO: Fix this naming, the returned coordinates are not really in clip space.
    //       They are in screen space alreay but retains the z value
    *(Out_Clip) = Vec3<float>(V0Screen_Vec4.x, V0Screen_Vec4.y, V0Screen_Vec4.z);
    *(Out_Clip + 1) = Vec3<float>(V1Screen_Vec4.x, V1Screen_Vec4.y, V1Screen_Vec4.z);
    *(Out_Clip + 2) = Vec3<float>(V2Screen_Vec4.x, V2Screen_Vec4.y, V2Screen_Vec4.z);

    Vec2<float> V0Screen(V0Screen_Vec4.x, V0Screen_Vec4.y);
    Vec2<float> V1Screen(V1Screen_Vec4.x, V1Screen_Vec4.y);
    Vec2<float> V2Screen(V2Screen_Vec4.x, V2Screen_Vec4.y);
   
    *(Out) = V0Screen;
    *(Out + 1) = V1Screen;
    *(Out + 2) = V2Screen;
}

void FragmentShader::Gouraud_Shader(Vec2<int> Fragment, 
                                    float Diffuse_Coef, 
                                    TGAImage& image,
                                    TGAColor Color)
{
    image.set(Fragment.x, Fragment.y, TGAColor(229 * Diffuse_Coef, 200 * Diffuse_Coef, 232 * Diffuse_Coef));
}

void FragmentShader::Toon_Shader(Vec2<int> Fragment, 
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
// @Param: Using directional_light for now thus no need for FragmentPos in world
void FragmentShader::Phong_Shader(Vec2<int> Fragment, 
                                  Vec3<float> n,
                                  Vec3<float> LightDir,
                                  Vec3<float> ViewDir,
                                  TGAImage& image,
                                  TGAColor MaterialColor,
                                  TGAColor Color,
                                  float ShadowCoef)
{
    float Toon_Threshold[5] = {
        0.1f,
        0.3f,
        0.5f,
        0.7f,
        0.9f
    };

    // Compute reflection light
    Vec3<float> ReflLightDir = n * 2.f * Math::DotProduct_Vec3(n, LightDir) - LightDir;
    float Diffuse_Coef = Math::DotProduct_Vec3(n, LightDir);
    float Specular_Coef = Math::DotProduct_Vec3(ViewDir, ReflLightDir);

    if (Diffuse_Coef < 0.f) Diffuse_Coef = 0.f;
    if (Specular_Coef < 0.f) Specular_Coef = 0.f;

    TGAColor Phong_Color;
    for (int i = 0; i < 3; i++) 
        Phong_Color[i] = std::min<float>(Ka * MaterialColor[i] + Color[i] * (Kd * Diffuse_Coef + Ks * std::pow(Specular_Coef, 10)), 255); 

    image.set(Fragment.x, Fragment.y, Phong_Color * ShadowCoef);
//    image.set(Fragment.x, Fragment.y, MaterialColor);
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

void FragmentShader::Shadow_Shader(Vec2<int> Fragment,
                                   TGAColor& Color,
                                   TGAImage& image)
{
    image.set(Fragment.x, Fragment.y, Color);
}

TGAColor FragmentShader::SampleTexture(TGAImage* TextureImage, 
                                       Vec3<float> Weights, 
                                       Vec2<float> V0_UV, 
                                       Vec2<float> V1_UV, 
                                       Vec2<float> V2_UV)
{
    Vec2<float> MappedTexturePos(
            Weights.x * V0_UV.x + Weights.y * V1_UV.x + Weights.z * V2_UV.x,
            Weights.x * V0_UV.y + Weights.y * V1_UV.y + Weights.z * V2_UV.y
    );

    TGAColor Color = TextureImage->get(
            TextureImage->get_width() * MappedTexturePos.x, 
            TextureImage->get_height() * MappedTexturePos.y
    );

    return Color;
}

// Global frame normal mapping
Vec3<float> FragmentShader::NormalMapping(TGAImage* NormalMap,
                                          Vec3<float> Weights,
                                          Vec2<float> V0_UV,
                                          Vec2<float> V1_UV,
                                          Vec2<float> V2_UV)
{
    TGAColor Color = this->SampleTexture(NormalMap, Weights, V0_UV, V1_UV, V2_UV); 
    // Remap the range since rgb is from [0, 255] while normal should be [-1, 1]
    Vec3<float> Normal;
    // TODO: Raw pixel layout is in bgra
    Normal.x = (Color[2] - (255.f / 2.f)) / (255.f / 2.f);
    Normal.y = (Color[1] - (255.f / 2.f)) / (255.f / 2.f);
    Normal.z = (Color[0] - (255.f / 2.f)) / (255.f / 2.f);
    return Normal;
}

// Tangent space normal mapping
Vec3<float> FragmentShader::NormalMapping_TangentSpace(TGAImage* NormalMap_TangentSpace,
                                                       Mat4x4<float> TBN,
                                                       Mat4x4<float> Model,
                                                       Vec3<float> Weights,
                                                       Vec2<float> V0_UV,
                                                       Vec2<float> V1_UV,
                                                       Vec2<float> V2_UV)
{
    Vec3<float> Normal_TangentSpace = this->NormalMapping(NormalMap_TangentSpace, Weights, V0_UV, V1_UV, V2_UV);
    Vec4<float> Normal_WorldSpace = TBN * Vec4<float>(Normal_TangentSpace, 0.f);
    Vec3<float> Result(Normal_WorldSpace.x, Normal_WorldSpace.y, Normal_WorldSpace.z);
    return Math::Normalize(Result);
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
                                       Vec3<float> Weights,
                                       float& FragmentDepth)
{
    int Index = ScreenY * 800 + ScreenX;

    /* Do the perspective correct interpolation here
        FragmentDepth = Weights.z / V0.z + Weights.x / V1.z + Weights.y / V2.z;
        FragmentDepth = 1.f / FragmentDepth;
    */
    // This may be faster to compute compare to the procedure above, need testing
    //FragmentDepth = (V0.z * V1.z * V2.z) / (Weights.z * V1.z * V2.z + Weights.x * V0.z * V2.z + Weights.y * V0.z * V1.z);
    FragmentDepth = 1 / (Weights.x / V0.z + Weights.y / V1.z + Weights.z / V2.z);

    if (FragmentDepth < ZBuffer[Index]) 
    {
        ZBuffer[Index] = FragmentDepth;
        return true;
    }

    return false;
}

bool FragmentShader::UpdateShadowBuffer(Vec3<float> V0, 
                                        Vec3<float> V1, 
                                        Vec3<float> V2, 
                                        int ScreenX, 
                                        int ScreenY, 
                                        Vec3<float> Weights,
                                        float& FragmentDepth)
{
    int Index = ScreenY * 800 + ScreenX;
    // TODO: Do the perspective correct interpolation here
    FragmentDepth = V0.z * Weights.z + V1.z * Weights.x + V2.z * Weights.y;

    // Note: Another caveat here with using clip space coordinates to do 
    //       depth test, since the z in clip space are all positive, because
    //       camera is looking at (0, 0, -1) this explains why using less than
    //       operator here
    if (FragmentDepth < ShadowBuffer[Index]) 
    {
        ShadowBuffer[Index] = FragmentDepth;
        return true;
    }
    return false;
}

Mat4x4<float> Shader::ConstructTBN(Vec3<float> V0_World, 
                                   Vec3<float> V1_World, 
                                   Vec3<float> V2_World,
                                   Vec2<float> V0_UV,
                                   Vec2<float> V1_UV,
                                   Vec2<float> V2_UV,
                                   Vec3<float> Surface_Normal)
{
    Mat4x4<float> Result;
    Vec3<float> t;
    Vec3<float> b;
    Vec3<float> T; // Tangent
    Vec3<float> B; // Bitangent
    Mat2x2<float> A;
    Vec3<float> P0P1 = V1_World - V0_World;
    Vec3<float> P0P2 = V2_World - V0_World;
    Vec2<float> Delta_UV0 = V1_UV - V0_UV;
    Vec2<float> Delta_UV1 = V2_UV - V0_UV;
    
    A.Mat[0][0] = Delta_UV0.x; // U1 - U0 
    A.Mat[0][1] = Delta_UV0.y; // V1 - V0
    A.Mat[1][0] = Delta_UV1.x; // U2 - U0
    A.Mat[1][1] = Delta_UV1.y; // V2 - V0

    // Invert A 
    Mat2x2<float> A_Inverse = A.Inverse();

    t.x = A_Inverse.Mat[0][0] * P0P1.x + A_Inverse.Mat[0][1] * P0P2.x;
    t.y = A_Inverse.Mat[0][0] * P0P1.y + A_Inverse.Mat[0][1] * P0P2.y;
    t.z = A_Inverse.Mat[0][0] * P0P1.z + A_Inverse.Mat[0][1] * P0P2.z;
    T = Math::Normalize(t);

    b.x = A_Inverse.Mat[1][0] * P0P1.x + A_Inverse.Mat[1][1] * P0P2.x;
    b.y = A_Inverse.Mat[1][0] * P0P1.y + A_Inverse.Mat[1][1] * P0P2.y;
    b.z = A_Inverse.Mat[1][0] * P0P1.z + A_Inverse.Mat[1][1] * P0P2.z;
    B = Math::Normalize(t);

    Result.Identity();

    // Set Column instead of row
    Result.SetColumn(0, Vec4<float>(T, 0.f));
    Result.SetColumn(1, Vec4<float>(B, 0.f));
    Result.SetColumn(2, Vec4<float>(Surface_Normal, 0.f));
    
    return Result;
}

void Shader::BackfaceCulling()
{

}

bool FragmentShader::IsInShadow(Vec4<float> Fragment_Model, 
                                Mat4x4<float>& Transform, 
                                Mat4x4<float>& Viewport,
                                float* DepthBuffer)
{
    Vec4<float> Fragment_Shadow = Transform * Fragment_Model;
    Fragment_Shadow.x = Fragment_Shadow.x / Fragment_Shadow.w;
    Fragment_Shadow.y = Fragment_Shadow.y / Fragment_Shadow.w;
    Fragment_Shadow.w = 1.f;
    Vec4<float> Fragment_ShadowScreen = Viewport * Fragment_Shadow;
    
    if (Fragment_ShadowScreen.x > 799.f) Fragment_ShadowScreen.x = 799.f; 
    if (Fragment_ShadowScreen.y > 799.f) Fragment_ShadowScreen.y = 799.f; 

    int ShadowBuffer_x = Fragment_ShadowScreen.x;
    int ShadowBuffer_y = Fragment_ShadowScreen.y;

    float ShadowCoef = 0.1f; 

    // TODO: Using magic number 0.015f here to work around z-fighting for now
    if (Fragment_Shadow.z > DepthBuffer[ShadowBuffer_y * 800 + ShadowBuffer_x] + 0.015f)
        return true;

    return false;
}

void Shader::DrawShadow(Model& Model, 
                        TGAImage& image, 
                        Vec3<float> LightPos, 
                        Vec3<float> LightDir, 
                        float* ShadowBuffer)
{
    // TODO: The refactor of Camera class may cause some bugs here
    // Do first pass rendering to decide which part of the mesh is visible
    Camera ShadowCamera;
    ShadowCamera.position = LightPos;
    // Need to negate the LightDir since LightDir reverted
    Mat4x4<float> View_Shadow = scene_manager.get_camera_view(ShadowCamera);
    // Cache the old MVP
    Mat4x4<float> Render_MVP = VS.MVP; 
    // same model, projection, viewport, but different view matrix
    FS.Shadow_MVP = VS.Projection * View_Shadow * VS.Model; 
    VS.MVP = FS.Shadow_MVP;

    Vec3<int>* IndexPtr = Model.Indices;
    int TriangleRendered = 0;

    while (TriangleRendered < Model.NumOfFaces)    
    {
        Vec4<float> V0_World_Augmented = VS.Model * Vec4<float>(Model.VertexBuffer[IndexPtr->x], 1.f);
        Vec4<float> V1_World_Augmented = VS.Model * Vec4<float>(Model.VertexBuffer[(IndexPtr + 1)->x], 1.f);
        Vec4<float> V2_World_Augmented = VS.Model * Vec4<float>(Model.VertexBuffer[(IndexPtr + 2)->x], 1.f);

        Vec3<float> V0_World(V0_World_Augmented.x, V0_World_Augmented.y, V0_World_Augmented.z);
        Vec3<float> V1_World(V1_World_Augmented.x, V1_World_Augmented.y, V1_World_Augmented.z);
        Vec3<float> V2_World(V2_World_Augmented.x, V2_World_Augmented.y, V2_World_Augmented.z);

        Vec3<float> V0V1 = V1_World - V0_World;
        Vec3<float> V0V2 = V2_World - V0_World;        

        Vec3<float> Surface_Normal = Math::Normalize(
                Math::CrossProduct(V0V1, V0V2));

        VS.Vertex_Shader(Model.VertexBuffer[IndexPtr->x],
                         Model.VertexBuffer[(IndexPtr + 1)->x],
                         Model.VertexBuffer[(IndexPtr + 2)->x], 
                         this->Triangle,
                         this->Triangle_Clip);

        Vec2<float> E1 = Triangle[1] - Triangle[0];
        Vec2<float> E2 = Triangle[2] - Triangle[0];
        float Denom = E1.x * E2.y - E2.x * E1.y;
        if (Denom == 0) continue;
        
        float bounds[4] = {
            Triangle[0].x, // Left
            Triangle[0].x, // Right
            Triangle[0].y, // Bottom
            Triangle[0].y  // Up
        };

        Math::bound_triangle(Triangle, bounds);

        for (int x = bounds[0]; x <= (int)bounds[1]; x++)
        {
            for (int y = bounds[2]; y <= (int)bounds[3]; y++)
            {
                Vec3<float> Weights = Math::barycentric(Triangle, x, y, Denom);
                // Point p is not inside of the triangle
                if (Weights.x < 0.f || Weights.y < 0.f || Weights.z < 0.f)
                    continue;

                float FragmentDepth = 0;
                
                // Depth test to see if current pixel is visible 
                if (this->FS.UpdateShadowBuffer(Triangle_Clip[0],      
                                                Triangle_Clip[1],
                                                Triangle_Clip[2],
                                                x, y, Weights, FragmentDepth))
                {
                    // TODO: Using magic number 1.f here
                    float Coef = (1.f - FragmentDepth) / 1.f;

                    // Shade the fragment
                    FS.Shadow_Shader(Vec2<int>(x, y), TGAColor(255 * Coef,
                                                               255 * Coef,
                                                               255 * Coef), image);
                }
            }
        }

        TriangleRendered++;
        IndexPtr += 3;
    }

    VS.MVP = Render_MVP;
}

void Shader::Draw(Model& Model, TGAImage& image, Camera& Camera, Shader_Mode ShadingMode)
{
    Vec3<float> LightDir(3.f, 0.f, 1.f);
    LightDir = Math::Normalize(LightDir);

    Vec3<int>* IndexPtr = Model.Indices;
    int TriangleRendered = 0;
    float Diffuse_Coefs[3];

    while (TriangleRendered < Model.NumOfFaces)    
    {
        Vec3<float> V0_Model = Model.VertexBuffer[IndexPtr->x];
        Vec3<float> V1_Model = Model.VertexBuffer[(IndexPtr + 1)->x];
        Vec3<float> V2_Model = Model.VertexBuffer[(IndexPtr + 2)->x];

        // TODO: Lighting related calculations are done in world space, so we need vertex information in 
        //       world space at each frame to derive the vertex normal, but this part still need to be cleaned up
        // Use vertex position in world to calculate surface normal and do backface culling
        Vec4<float> V0_World_Augmented = VS.Model * Vec4<float>(Model.VertexBuffer[IndexPtr->x], 1.f);
        Vec4<float> V1_World_Augmented = VS.Model * Vec4<float>(Model.VertexBuffer[(IndexPtr + 1)->x], 1.f);
        Vec4<float> V2_World_Augmented = VS.Model * Vec4<float>(Model.VertexBuffer[(IndexPtr + 2)->x], 1.f);

        Vec3<float> V0_World(V0_World_Augmented.x, V0_World_Augmented.y, V0_World_Augmented.z);
        Vec3<float> V1_World(V1_World_Augmented.x, V1_World_Augmented.y, V1_World_Augmented.z);
        Vec3<float> V2_World(V2_World_Augmented.x, V2_World_Augmented.y, V2_World_Augmented.z);

        VS.Vertex_Shader(Model.VertexBuffer[IndexPtr->x],      
                         Model.VertexBuffer[(IndexPtr + 1)->x],
                         Model.VertexBuffer[(IndexPtr + 2)->x],
                         this->Triangle,
                         this->Triangle_Clip);

        // TODO: Backface culling should be done view space
        BackfaceCulling();

        // TODO: backface culling should use camera space coordinates or clip space
        Vec3<float> V0V1 =  V1_World - V0_World;
        Vec3<float> V0V2 =  V2_World - V0_World;

        // Calculate diffuse coef at each vertex here
        Diffuse_Coefs[0] = Math::DotProduct_Vec3(
                            Model.VertexNormalBuffer[IndexPtr->z], LightDir 
                );

        Diffuse_Coefs[1] = Math::DotProduct_Vec3(
                            Model.VertexNormalBuffer[(IndexPtr + 1)->z], LightDir
                );
        
        Diffuse_Coefs[2] = Math::DotProduct_Vec3(
                            Model.VertexNormalBuffer[(IndexPtr + 2)->z], LightDir 
                );

        // Derive surface normal, counter-clock wise winding order
        Vec3<float> Surface_Normal = Math::Normalize(
                Math::CrossProduct(V0V1, V0V2));

        // in screen space
        // (V1.x - V0.x) * (V2.y - V0.y) == (V2.x - V0.x) * (V1.y - V0.y)
        Vec2<float> E1 = Triangle[1] - Triangle[0];
        Vec2<float> E2 = Triangle[2] - Triangle[0];
        float Denom = E1.x * E2.y - E2.x * E1.y;
        if (Denom == 0) return;
        
        Vec2<float> V0_UV = Model.TextureBuffer[IndexPtr->y];
        Vec2<float> V1_UV = Model.TextureBuffer[(IndexPtr + 1)->y];
        Vec2<float> V2_UV = Model.TextureBuffer[(IndexPtr + 2)->y];

        // Construct the TBN matrix for later normal mapping
        Mat4x4<float> TBN = ConstructTBN(V0_World, V1_World, V2_World, V0_UV, V1_UV, V2_UV, Surface_Normal);

        float bounds[4] = {
            Triangle[0].x, // Left
            Triangle[0].x, // Right
            Triangle[0].y, // Bottom
            Triangle[0].y  // Up
        };

        Math::bound_triangle(Triangle, bounds);

        for (int x = bounds[0]; x <= (int)bounds[1]; x++)
        {
            for (int y = bounds[2]; y <= (int)bounds[3]; y++)
            {
                Vec3<float> Weights = Math::barycentric(Triangle, x, y, Denom);
                // Point p is not inside of the triangle
                if (Weights.x < 0.f || Weights.y < 0.f || Weights.z < 0.f)
                    continue;

                float FragmentDepth = 0;

                // Depth test to see if current pixel is visible 
                if (this->FS.UpdateDepthBuffer(Triangle_Clip[0],
                                               Triangle_Clip[1],
                                               Triangle_Clip[2],
                                               x, y, Weights,
                                               FragmentDepth))
                {
                    switch (ShadingMode)
                    {
                        case Shader_Mode::Flat_Shader:
                        {
                            FS.Fragment_Shader(Vec2<int>(x, y),
                                               V0_UV, 
                                               V1_UV,
                                               V2_UV,
                                               Weights,
                                               Model.TextureAssets[0], 
                                               image);
                            break;
                        }

                        case Shader_Mode::Gouraud_Shader:
                        {
                            float Diffuse_Coef = Weights.x * Diffuse_Coefs[0] + Weights.y * Diffuse_Coefs[1] + Weights.z * Diffuse_Coefs[2];

                            if (Diffuse_Coef > 1.f) Diffuse_Coef = 1.f;
                            if (Diffuse_Coef < 0.f) Diffuse_Coef = 0.f;

                            FS.Gouraud_Shader(Vec2<int>(x, y), Diffuse_Coef, image, TGAColor(255, 255, 255));
                            break;
                        }

                        case Shader_Mode::Phong_Shader:
                        {
                            // Interpolate normal and feed to fragment shader
                            Vec3<float> V0_Normal = Model.VertexNormalBuffer[IndexPtr->z];
                            Vec3<float> V1_Normal = Model.VertexNormalBuffer[(IndexPtr + 1)->z];
                            Vec3<float> V2_Normal = Model.VertexNormalBuffer[(IndexPtr + 2)->z];

                            // Interpolate normals
                            Vec3<float> Interpolated_Normal(Weights.x * V0_Normal.x + Weights.y * V1_Normal.x + Weights.z * V2_Normal.x,
                                                            Weights.x * V0_Normal.y + Weights.y * V1_Normal.y + Weights.z * V2_Normal.y,
                                                            Weights.x * V0_Normal.z + Weights.y * V1_Normal.z + Weights.z * V2_Normal.z);

                            Vec3<float> n = Math::Normalize(Interpolated_Normal);

                            // Use interpolated normal at each fragment instead of 
                            // flat surface normal for smoothing out the 
                            // edges between shaded triangles
                            TBN.SetColumn(2, Vec4<float>(n, 0.f));

                            // TODO: Average the tangent at each vertex  
                           


                            // -----------------------------------------
                            
                            Vec3<float> Normal = FS.NormalMapping_TangentSpace(Model.NormalTexture, 
                                                                               TBN, 
                                                                               VS.Model, 
                                                                               Weights, 
                                                                               V0_UV, 
                                                                               V1_UV, 
                                                                               V2_UV);

                            TGAColor Color = FS.SampleTexture(Model.TextureAssets[0],
                                                              Weights, 
                                                              V0_UV,
                                                              V1_UV, 
                                                              V2_UV);

                            // TODO: clean this up 
                            Vec4<float> Fragment_Model;
                            Fragment_Model.x = Weights.x * V0_Model.x + Weights.y * V1_Model.x + Weights.z * V2_Model.x;
                            Fragment_Model.y = Weights.x * V0_Model.y + Weights.y * V1_Model.y + Weights.z * V2_Model.y;
                            Fragment_Model.z = Weights.x * V0_Model.z + Weights.y * V1_Model.z + Weights.z * V2_Model.z;
                            Fragment_Model.w = 1.f;

                            Vec4<float> Fragment_World = VS.Model * Fragment_Model;

                            // Shadow mapping
                            //float ShadowCoef = FS.IsInShadow(Fragment_Model, FS.Shadow_MVP, VS.Viewport, FS.ShadowBuffer) ? 0.6f : 0.f;
                            float shadow_coef = 1.f;
                            // TODO: Need to swap out the hard-coded viewing direction later
                            FS.Phong_Shader(Vec2<int>(x, y), Normal, LightDir, Math::Normalize(Vec3<float>(1.f, .5f, 1.f)), image, Color, TGAColor(200, 200, 200), shadow_coef);
                            
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

void Shader::draw_cubemap(Cubemap& cubemap, TGAImage& image, struct Camera& camera) {

}

bool UpdateDepthBuffer(Vec3<float> V0, 
                       Vec3<float> V1, 
                       Vec3<float> V2, 
                       int ScreenX, 
                       int ScreenY, 
                       Vec3<float> Weights,
                       float& FragmentDepth,
                       float* DepthBuffer)
{
    int Index = ScreenY * 800 + ScreenX;
    // TODO: Do the perspective correct interpolation here
    FragmentDepth = V0.z * Weights.z + V1.z * Weights.x + V2.z * Weights.y;

    if (FragmentDepth < DepthBuffer[Index]) 
    {
        DepthBuffer[Index] = FragmentDepth;
        return true;
    }

    return false;
}


// TODO: change the internal representation of u,v,w so that u is for v0, v is for v1, and w is for v2
// Generate the occlusion texture
void Shader::DrawOcclusion(Model& Model, TGAImage& occlusion_texture, float* occlusion_depth_buffer)
{
    Vec3<int>* IndexPtr = Model.Indices;
    int TriangleRendered = 0;

    while (TriangleRendered < Model.NumOfFaces)    
    {
        VS.Vertex_Shader(Model.VertexBuffer[IndexPtr->x],      
                         Model.VertexBuffer[(IndexPtr + 1)->x],
                         Model.VertexBuffer[(IndexPtr + 2)->x],
                         this->Triangle,
                         this->Triangle_Clip);

        // in screen space
        // (V1.x - V0.x) * (V2.y - V0.y) == (V2.x - V0.x) * (V1.y - V0.y)
        Vec2<float> E1 = Triangle[1] - Triangle[0];
        Vec2<float> E2 = Triangle[2] - Triangle[0];
        float Denom = E1.x * E2.y - E2.x * E1.y;
        if (Denom == 0) return;
        
        float bounds[4] = {
            Triangle[0].x, // Bottom
            Triangle[0].x, // Top
            Triangle[0].y, // Left
            Triangle[0].y  // Right
        };

        Math::bound_triangle(Triangle, bounds);

        for (int x = bounds[0]; x <= (int)bounds[1]; x++)
        {
            for (int y = bounds[2]; y <= (int)bounds[3]; y++)
            {
                Vec3<float> Weights = Math::barycentric(Triangle, x, y, Denom);
                if (Weights.x < 0.f || Weights.y < 0.f || Weights.z < 0.f) continue;
                float fragment_depth = 0;
                // Depth test to see if current pixel is visible 
                if (UpdateDepthBuffer(Triangle_Clip[0],
                                      Triangle_Clip[1],
                                      Triangle_Clip[2],
                                      x, y, Weights,
                                      fragment_depth,
                                      occlusion_depth_buffer))
                {
                    Vec2<float> uv0 = Model.TextureBuffer[IndexPtr->y];
                    Vec2<float> uv1 = Model.TextureBuffer[(IndexPtr + 1)->y];
                    Vec2<float> uv2 = Model.TextureBuffer[(IndexPtr + 2)->y];
                    Vec2<float> uv;  
                    uv.x = Weights.z * uv0.x + Weights.x * uv1.x + Weights.y * uv2.x;
                    uv.y = Weights.z * uv0.y + Weights.x * uv1.y + Weights.y * uv2.y;
                    occlusion_texture.set(uv.x * 1024, uv.y * 1024, TGAColor(
                                255, 
                                255, 
                                255));
                }
            }
        }

        IndexPtr += 3;    
        TriangleRendered++;
    }
}

void Shader_Base::initFragmentAttrib(uint32_t bufferWidth, uint32_t bufferHeight) {
    bufferWidth_ = bufferWidth;
    bufferHeight_ = bufferHeight;
    fragmentAttribBuffer = new FragmentAttrib[bufferWidth * bufferHeight];
}

void Shader_Base::set_model_matrix(Mat4x4<float>& model) {
    model_ = model;
}

void Shader_Base::set_view_matrix(Mat4x4<float>& view) {
    view_ = view;
}

void Shader_Base::set_projection_matrix(Mat4x4<float>& projection) {
    projection_ = projection;
}

void Shader_Base::bindTexture(Texture* texture) {
    texture_ = texture;
}

Phong_Shader::Phong_Shader() {
    textureSampler = nullptr;
}

Vec4<float> Phong_Shader::vertex_shader(Vec3<float>& v) {
    return projection_ * view_ * model_ * Vec4<float>(v, 1.f);    
}

// Fragment normal
// Fragment textureUV
// Fragment lightDirection
// Fragment viewDirection
Vec4<float> Phong_Shader::fragment_shader(int x, int y) {
    FragmentAttrib& attribs = fragmentAttribBuffer[y * bufferWidth_ + x];
    Vec4<float> fragmentColor(0.f, 0.f, 0.f, 1.f);

    // Ambient + Diffuse + Specular
    //Vec3<float> ambient();
    //Vec3<float> diffuse(0.f, 0.f, 0.f);

    //diffuse =  
    Vec3<float> ambient = textureSampler->sampleTexture2D(*texture_, attribs.textureCoord.x, attribs.textureCoord.y);

    return fragmentColor;
}

Vec3<float> TextureSampler::sampleTexture2D(Texture& texture, float u, float v) {
    Vec3<float> res;
    return res;
}