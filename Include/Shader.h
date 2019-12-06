#pragma once

#include "tgaimage.h"
#include "Math.h"
#include "Model.h"
#include "scene.h"

struct FragmentAttrib {
    Vec3<float> textureCoord;
    Vec3<float> normal;
    Vec3<float> tangent;
};

struct LightParams {
    Vec3<float> viewSpaceFragmentPos;
};

class ShaderBase {
public:
    // vertex in
    Mat4x4<float> model_;
    Mat4x4<float> view_;
    Mat4x4<float> modelView_;
    Mat4x4<float> projection_;

    Vec3<float> cameraPos;
    std::vector<DirectionalLight> dirLights;

    uint8_t vertexAttribFlag;
    // per fragment attrib
    FragmentAttrib* fragmentAttribBuffer;
    LightParams* lightParamsBuffer;
    uint32_t bufferWidth_, bufferHeight_;

    std::vector<Texture*> diffuseMaps;
    std::vector<Texture*> specularMaps;
    Texture* normalMap_;     
    Texture* aoMap_;
    Texture* roughnessMap_;

    ShaderBase();
    void set_model_matrix(Mat4x4<float>& model);
    void set_view_matrix(Mat4x4<float>& view);
    void set_projection_matrix(Mat4x4<float>& projection);
    void clearFragmentAttribs();
    void bindDiffuseTexture(Texture* texture);
    void bindSpecTexture(Texture* texture);
    void unbindTexture();

    virtual Vec4<float> vertexShader(Vec3<float>& v) = 0;
    virtual Vec4<int> fragmentShader(int x, int y) = 0;
    void initFragmentAttrib(uint32_t bufferWidth, uint32_t bufferHeight);
    Vec3<float> sampleTexture2D(Texture& texture, float u, float v);
    Vec3<float> sampleNormal(Texture& normalMap, float u, float v);
    float sampleGreyScale(Texture& textureMap, float u, float v);
    Vec3<float> transformNormal(Vec3<float>& normal);
    Vec3<float> transformTangent(Vec3<float>& tangent);
    Vec3<float> transformToViewSpace(Vec3<float>& v);
};

class PhongShader : public ShaderBase {
    // Need
    // @ vertex normal
    // @ texture uv
    // @ diffuse map
    // @ specular map
    // @ view vector
public:
    PhongShader();
    Vec4<float> vertexShader(Vec3<float>& v) override;
    Vec4<int> fragmentShader(int x, int y) override;
};

class SkyboxShader : public ShaderBase {
public:
    Texture* texture_[6];
    Vec4<float> vertexShader(Vec3<float>& v) override;
    Vec4<int> fragmentShader(int x, int y) override;
};

class DepthShader : public ShaderBase {
    Vec4<float> vertexShader(Vec3<float>& v) override;
    Vec4<int> fragmentShader(int x, int y) override;
};

class PBRShader : public ShaderBase {
    Vec4<float> vertexShader(Vec3<float>& v) override;
    Vec4<int> fragmentShader(int x, int y) override;
}; 

