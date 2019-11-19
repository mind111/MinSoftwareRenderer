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
    std::vector<Shader_Base*> shader_list; // shader pool that include all available style of shaders
    SkyboxShader* skyboxShader_;

    // This maybe refactored to be part of rasterizer
    Vec4<float> triangleView[3];
    Vec4<float> triangle_clip[3];
    Vec2<float> triangle_screen[3];
    Vec3<float> triangle_uv[3]; // if uv only has two components then fill z with 0
    Vec3<float> normalIn[3];
    Vec3<float> normalOut[3];
    Vec3<float> tangentIn[3];
    Vec3<float> tangentOut[3];

    uint8_t mesh_attrib_flag;

    Shader_Base* activeShaderPtr_;
    Mat4x4<float> viewport;
    unsigned char* backbuffer;
    
    uint32_t buffer_width, buffer_height;

    Renderer();
    void init();
    void alloc_backbuffer(Window& window);
    void bind_mesh_buffers(Mesh& mesh);
    void draw_pixel(int x, int y, Vec4<int>& color);
    void drawScene(Scene& scene);
    void drawSkybox(Scene& scene);
    void draw_instance(Light* light, Mesh& mesh);
    void drawLine(Vec2<int> start, Vec2<int> end);
    void drawTangents(Vec3<float>& vertexPos, Vec3<float>& tangent) {
        Vec3<float> end = vertexPos + tangent * 0.03f;
        Vec4<float> vertexClip = activeShaderPtr_->vertex_shader(vertexPos);
        Vec4<float> vertexScreen = viewport * (vertexClip / vertexClip.w);
        Vec2<int> startScreen(vertexScreen.x, vertexScreen.y);
        Vec4<float> endClip = activeShaderPtr_->vertex_shader(end);
        Vec4<float> endScreenVec4 = viewport * (endClip / endClip.w);
        Vec2<int> endScreen(endScreenVec4.x, endScreenVec4.y);
        drawLine(startScreen, endScreen);
    }
    void clearBuffer();
    void clearDepth();

private:
    float* z_buffer;
    void draw_mesh(Mesh& mesh);
    void perspectiveCorrection(Vec3<float>& baryCoord);
    bool depthTest(int x, int y, Vec3<float> baryCoord);
    void fill_triangle(Shader_Base* active_shader_ptr, Light* light);
};

