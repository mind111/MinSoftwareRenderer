#include <chrono>
#include "Shader.h"
#include "scene.h"
#include "window.h"

// TODO: @ Benchmarking
class PerformanceTimer {
    using clock = std::chrono::high_resolution_clock;
public:
    PerformanceTimer() {
        start = clock::now(); 
    }
    ~PerformanceTimer() {
        end = clock::now();
        auto startTimepoint = std::chrono::time_point_cast<std::chrono::milliseconds>(start).time_since_epoch().count();
        auto endTimepoint = std::chrono::time_point_cast<std::chrono::milliseconds>(end).time_since_epoch().count();
        std::cout << "Time elapsed: " << endTimepoint - startTimepoint << "ms" << std::endl;
    }

private:
    clock::time_point start;
    clock::time_point end; 
};

class Rasterizer {

};

class Renderer {
public:
    std::vector<ShaderBase*> shader_list; // shader pool that include all available style of shaders
    SkyboxShader* skyboxShader_;

    uint8_t mesh_attrib_flag;
    ShaderBase* activeShaderPtr_;
    Mat4x4<float> viewport;
    unsigned char* backbuffer;
    
    uint32_t bufferWidth_, bufferHeight_;

    Renderer();
    void init();
    void alloc_backbuffer(Window& window);
    void bind_mesh_buffers(Mesh& mesh);
    void draw_pixel(int x, int y, Vec4<int>& color);
    void drawScene(Scene& scene);
    void drawSkybox(Scene& scene);
    void drawInstance(Mesh& mesh);
    void drawLine(Vec2<int> start, Vec2<int> end);
    void drawTriangleWireFrame(Vec2<int> v0, Vec2<int> v1, Vec2<int> v2);
    void drawTangents(Vec3<float>& vertexPos, Vec3<float>& tangent) {
        Vec3<float> end = vertexPos + tangent * 0.1f;
        Vec4<float> vertexClip = activeShaderPtr_->vertexShader(vertexPos);
        Vec4<float> vertexScreen = viewport * (vertexClip / vertexClip.w);
        Vec2<int> startScreen(vertexScreen.x, vertexScreen.y);
        Vec4<float> endClip = activeShaderPtr_->vertexShader(end);
        Vec4<float> endScreenVec4 = viewport * (endClip / endClip.w);
        Vec2<int> endScreen(endScreenVec4.x, endScreenVec4.y);
        drawLine(startScreen, endScreen);
    }
    void clearBuffer(Vec3<int>& clearColor);
    void clearDepth();
    void clearNormalBuffer();
    //-----------------
    Vec3<float> reconstructViewPosFromDepth(int x, int y, float depth);
    void sampleOcclusion(int x, int y, Vec2<float> dir);
    void SSAO();

private:
    float* zBuffer;
    Vec3<float>* normalBuffer;
    void draw_mesh(Mesh& mesh);
    void drawDebugLines(Mesh&, Vec3<float>* vPosView, uint32_t f_idx);
    bool backfaceCulling(Vec3<float>* vPosView);
    void perspectiveCorrection(Vec3<float>* vPosView, Vec3<float>& baryCoord);
    bool depthTest(int x, int y, Vec3<float> baryCoord, Vec3<float>* vPosScreen);
    void fillTriangle(Vec3<float>* vPosScreen, Vec3<float>* vPosView, Vec3<float>* normals, Vec3<float>* textureCoords, Vec3<float>* tangents);
    float doAmbientOcclusion(int x, int y, Vec2<float>& sampleDir);
    float ambientOcclusion(int x, int y, Vec2<float>& sampleDir);
    void bindMeshTextures(Scene& scene, const Mesh& mesh);
};

