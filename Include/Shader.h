#pragma once

#include "tgaimage.h"
#include "Math.h"
#include "Model.h"
#include "scene.h"

enum class Shader_Mode : int8_t
{
    Flat_Shader,
    Gouraud_Shader,
    Phong_Shader,
    Toon_Shader
};

struct FragmentAttrib {
    Vec3<float> textureCoord;
    Vec3<float> normal;
    Vec3<float> tangent;
};

struct LightingParams {
    Vec3<float> color;
    Vec3<float> direction;
    Vec3<float> viewSpaceFragmentPos;
    float intensity;
};

class Shader_Base {
public:
    // vertex in
    Mat4x4<float> model_;
    Mat4x4<float> view_;
    Mat4x4<float> modelView_;
    Mat4x4<float> projection_;

    Vec3<float> cameraPos;

    uint8_t vertexAttribFlag;
    // per fragment attrib
    FragmentAttrib* fragmentAttribBuffer;
    LightingParams* lightingParamBuffer;
    uint32_t bufferWidth_, bufferHeight_;

    std::vector<Texture*> diffuseMaps;
    std::vector<Texture*> specularMaps;
    Texture* normalMap_;
    
    Shader_Base();
    void set_model_matrix(Mat4x4<float>& model);
    void set_view_matrix(Mat4x4<float>& view);
    void set_projection_matrix(Mat4x4<float>& projection);
    void clearFragmentAttribs();
    void bindDiffuseTexture(Texture* texture);
    void bindSpecTexture(Texture* texture);
    void unbindTexture();

    virtual Vec4<float> vertex_shader(Vec3<float>& v) = 0;
    virtual Vec4<int> fragment_shader(int x, int y) = 0;
    void initFragmentAttrib(uint32_t bufferWidth, uint32_t bufferHeight);
    Vec3<float> sampleTexture2D(Texture& texture, float u, float v);
    Vec3<float> sampleNormal(Texture& normalMap, float u, float v);
    Vec3<float> transformNormal(Vec3<float>& normal);
    Vec3<float> transformTangent(Vec3<float>& tangent);
    Vec3<float> transformToViewSpace(Vec3<float>& v);
};

class Phong_Shader : public Shader_Base {
    // Need
    // @ vertex normal
    // @ texture uv
    // @ diffuse map
    // @ specular map
    // @ view vector
public:
    Phong_Shader();
    Vec4<float> vertex_shader(Vec3<float>& v) override;
    Vec4<int> fragment_shader(int x, int y) override;
};

class SkyboxShader : public Shader_Base {
public:
    Texture* texture_[6];
    Vec4<float> vertex_shader(Vec3<float>& v) override;
    Vec4<int> fragment_shader(int x, int y) override;
};
