#pragma once

#include "tgaimage.h"
#include "Math.h"
#include "Model.h"

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
    void Gouraud_Shader();
    void Fragment_Shader(int ScreenX, int ScreenY);
    TGAColor SampleTexture(TGAImage* TextureImage, 
                           Vec3<float> Weights, 
                           Vec2<float> V0_UV, 
                           Vec2<float> V1_UV, 
                           Vec2<float> V2_UV);
};

struct Shader
{
    Vec2<float> Triangle[3]; // Triangle went through vertex shader and waiting for shading
    int NumOfTriangles;
    float* ZBuffer;
        
    VertexShader VS;
    FragmentShader FS; 
     
    bool UpdateDepthBuffer(Vec3<float> V0, Vec3<float> V1,
                           Vec3<float> V2, int ScreenX, 
                           int ScreenY, Vec3<float> Weights);

    void Draw(Model& Model, TGAImage& image, float* ZBuffer);
};