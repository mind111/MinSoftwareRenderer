#pragma once

#include "tgaimage.h"
#include "Math.h"
#include "Model.h"

enum class Shader_Mode : int8_t
{
    Flat_Shader,
    Gouraud_Shader,
    Phong_Shader,
    Toon_Shader
};

struct VertexShader
{
    Mat4x4<float> Viewport;
    Mat4x4<float> MVP;    

    Vec3<float>* VertexBuffer;

    void Vertex_Shader(Vec3<float> V0,
                         Vec3<float> V1,
                         Vec3<float> V2,
                         Vec2<float>* Out);
};

struct FragmentShader
{
    float* ZBuffer;

    bool UpdateDepthBuffer(Vec3<float> V0, Vec3<float> V1,
                           Vec3<float> V2, int ScreenX, 
                           int ScreenY, Vec3<float> Weights);

    void Gouraud_Shader(Vec2<int> Fragment,
                        float Diffuse_Coef,
                        TGAImage& image,
                        TGAColor Color);

    void Toon_Shader(Vec2<int> Fragment,
                     float Diffuse_Coef,
                     TGAImage& image,
                     TGAColor Color);

    void Phong_Shader(Vec2<int> Fragment, 
                      Vec3<float> Normal,
                      Vec3<float> LightDir,
                      Vec3<float> ViewDir,
                      TGAImage& image,
                      TGAColor MaterialColor,
                      TGAColor Color);

    void Fragment_Shader(Vec2<int> Fragment, 
                         Vec2<float> V0_UV,
                         Vec2<float> V1_UV, 
                         Vec2<float> V2_UV,
                         Vec3<float>& Weights, 
                         TGAImage* TextureAsset, 
                         TGAImage& image); 

    TGAColor SampleTexture(TGAImage* TextureImage, 
                           Vec3<float> Weights, 
                           Vec2<float> V0_UV, 
                           Vec2<float> V1_UV, 
                           Vec2<float> V2_UV);

    Vec3<float> NormalMapping(TGAImage* NormalMap,
                              Vec3<float> Weights,
                              Vec2<float> V0_UV,
                              Vec2<float> V1_UV,
                              Vec2<float> V2_UV);
};

struct Shader
{
    int NumOfTriangles;
    Vec2<float> Triangle[3]; // Triangle went through vertex shader and waiting for shading 

    VertexShader VS;
    FragmentShader FS; 
     
    void Draw(Model& Model, TGAImage& image, struct Camera& Camera, Shader_Mode ShadingMode);
};
