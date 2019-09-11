#pragma once

#include "tgaimage.h"
#include "Math.h"
#include "Model.h"

struct VertexShader
{
    Mat4x4<float> Viewport;
    Mat4x4<float> MVP;    

    Vec3<float>* VertexBuffer;

    void ProcessTriangle(Vec3<float> V0,
                         Vec3<float> V1,
                         Vec3<float> V2,
                         Vec2<float>* Out);
};

struct FragmentShader
{
    bool UpdateDepthBuffer(Vec3<float> V0, Vec3<float> V1,
                                           Vec3<float> V2, int ScreenX, 
                                           int ScreenY, Vec3<float> Weights, 
                                           float* ZBuffer);

    void RasterizeTriangle(Vec2<float> V0Screen, Vec2<float> V1Screen, 
        Vec2<float> V2Screen, 
        Vec3<float> V0_World,
        Vec3<float> V1_World,
        Vec3<float> V2_World,
        TGAImage& image, 
        Vec2<float>& V0_UV, 
        Vec2<float>& V1_UV, 
        Vec2<float>& V2_UV, 
        TGAImage* TextureImage,
        float* ZBuffer);
};

struct Shader
{
    Vec2<float> Triangle[3]; // Triangle went through vertex shader and waiting for shading
    int NumOfTriangles;
    float* ZBuffer;
        
    VertexShader VS;
    FragmentShader FS; 
     
    void Draw(Model& Model, TGAImage& image, float* ZBuffer)
    {
        Vec3<int>* IndexPtr = Model.Indices;
        int TriangleRendered = 0;

        while (TriangleRendered < 2492)
        {

            Vec3<float> V0V1 =  Model.VertexBuffer[(IndexPtr + 1)->x] - Model.VertexBuffer[IndexPtr->x];
            Vec3<float> V0V2 =  Model.VertexBuffer[(IndexPtr + 2)->x] - Model.VertexBuffer[IndexPtr->x];

            ///Note: Counter-clockwise vertex winding order
            Vec3<float> Normal = MathFunctionLibrary::Normalize(MathFunctionLibrary::CrossProduct(V0V1, V0V2));

            float ShadingCoef = MathFunctionLibrary::DotProduct_Vec3(Vec3<float>(0, 0, 3), Normal);
            // ShadingCoef < 0 means that the triangle is facing away from the light, simply discard
            if (ShadingCoef < 0.0f) 
            {
                TriangleRendered++;
                IndexPtr += 3;
                continue;
            }
            
            if (ShadingCoef > 1.0f) ShadingCoef = 1.0f;

            VS.ProcessTriangle(Model.VertexBuffer[IndexPtr->x],      
                               Model.VertexBuffer[(IndexPtr + 1)->x],
                               Model.VertexBuffer[(IndexPtr + 2)->x],
                               Triangle);

            Vec2<float> V0_UV = Model.TextureBuffer[IndexPtr->y];
            Vec2<float> V1_UV = Model.TextureBuffer[(IndexPtr + 1)->y];
            Vec2<float> V2_UV = Model.TextureBuffer[(IndexPtr + 2)->y];

            FS.RasterizeTriangle(Triangle[0], Triangle[1], Triangle[2],
                                 Model.VertexBuffer[IndexPtr->x],
                                 Model.VertexBuffer[(IndexPtr + 1)->x],
                                 Model.VertexBuffer[(IndexPtr + 2)->x],
                                 image, 
                                 V0_UV,
                                 V1_UV,
                                 V2_UV, 
                                 Model.TextureAssets[0],
                                 this->ZBuffer);
            IndexPtr += 3;
            TriangleRendered++;
        }
    }
};
