#pragma once

#include <map>
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

class Shader_Base {
public:
    Mat4x4<float> model;
    Mat4x4<float> view;
    Mat4x4<float> projection;
    
    void set_model_matrix(Mat4x4<float>& model);
    void set_view_matrix(Mat4x4<float>& view);
    void set_projection_matrix(Mat4x4<float>& projection);

    virtual Vec4<float> vertex_shader(Vec3<float>& v) = 0;
    virtual Vec4<float> fragment_shader(int x, int y) = 0;  
};

class Texture_Sampler {

};

class Phong_Shader : public Shader_Base {
    // Need
    // @ vertex normal
    // @ texture uv
    // @ diffuse map
    // @ specular map
    // @ view vector
public:
    Texture_Sampler* texture_sampler;

    Phong_Shader();
    Vec4<float> vertex_shader(Vec3<float>& v) override;
    Vec4<float> fragment_shader(int x, int y) override;
};


struct VertexShader
{
    Mat4x4<float> Model;
    Mat4x4<float> Projection;
    Mat4x4<float> MVP;    
    Mat4x4<float> Viewport;

    Vec3<float>* VertexBuffer;

    void Vertex_Shader(Vec3<float> V0,
                       Vec3<float> V1,
                       Vec3<float> V2,
                       Vec2<float>* Out,
                       Vec3<float>* Out_Clip);
};

struct FragmentShader
{
    float* ZBuffer;
    float* ShadowBuffer;
    float* AmbientBuffer;

    Mat4x4<float> Shadow_MVP;

    bool UpdateDepthBuffer(Vec3<float> V0, 
                           Vec3<float> V1,
                           Vec3<float> V2, 
                           int ScreenX, 
                           int ScreenY, 
                           Vec3<float> Weights,
                           float& FragmentDepth);

    bool UpdateShadowBuffer(Vec3<float> V0, 
                            Vec3<float> V1, 
                            Vec3<float> V2, 
                            int ScreenX, 
                            int ScreenY, 
                            Vec3<float> Weights,
                            float& FragmentDepth);

    bool IsInShadow(Vec4<float> Fragment_Model, 
                    Mat4x4<float>& Transform, 
                    Mat4x4<float>& Viewport,
                    float* DepthBuffer);

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
                      TGAColor Color,
                      float ShadowCoef);

    void Fragment_Shader(Vec2<int> Fragment, 
                         Vec2<float> V0_UV,
                         Vec2<float> V1_UV, 
                         Vec2<float> V2_UV,
                         Vec3<float>& Weights, 
                         TGAImage* TextureAsset, 
                         TGAImage& image); 

    void Shadow_Shader(Vec2<int> Fragment,
                       TGAColor& Color,
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

    Vec3<float> NormalMapping_TangentSpace(TGAImage* NormalMap_TangentSpace,
                                           Mat4x4<float> TBN,
                                           Mat4x4<float> Model,
                                           Vec3<float> Weights,
                                           Vec2<float> V0_UV,
                                           Vec2<float> V1_UV,
                                           Vec2<float> V2_UV);
};

struct Shader
{
    int NumOfTriangles;
    Vec3<float> Triangle_Clip[3]; // Triangle went through vertex shader and waiting for shading 
    Vec2<float> Triangle[3];

    VertexShader VS;
    FragmentShader FS; 
     
    Mat4x4<float> ConstructTBN(Vec3<float> V0_World, 
                               Vec3<float> V1_World, 
                               Vec3<float> V2_World,
                               Vec2<float> V0_UV,
                               Vec2<float> V1_UV,
                               Vec2<float> V2_UV,
                               Vec3<float> Surface_Normal);

    void BackfaceCulling();
    void DrawOcclusion(Model& Model, TGAImage& occlusion_texture, float* occlustion_depth_buffer);
    void DrawShadow(Model& Model, TGAImage& image, Vec3<float> LightPos, Vec3<float> LightDir, float* ShadowBuffer);
    void draw_cubemap(Cubemap& cubemap, TGAImage& image, struct Camera& camera);
    void Draw(Model& Model, TGAImage& image, struct Camera& Camera, Shader_Mode ShadingMode);
};
