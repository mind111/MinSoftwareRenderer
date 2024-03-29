#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include "tgaimage.h"
#include "globals.h"
#include "model.h"
#include "window.h"
#include "scene.h"
#include "renderer.h"
#include "shader.h"

/// \Note: Using naive scan-line method
void FillTriangle(Vec2<int>& V0, Vec2<int>& V1, Vec2<int>& V2, TGAImage& image, const TGAColor& color)
{
    // Sort the vertices according to their y value
    if (V0.y > V1.y) V0.Swap(V1);
    if (V1.y > V2.y) V1.Swap(V2);

    /// \Note: Compress code for rasterizing bottom half and upper half into one chunk
    for (int y = V0.y; y < V2.y; y++)
    {
         bool UpperHalf = (y >= V1.y);
        // Triangle similarity
        /// \Note: Speed-up: extract the constant part of the formula, 
        ///  the only variable in this calculation that is changing during
        ///  every iteration is y
        int Left = (V2.x - V0.x) * (y - V0.y) / (V2.y - V0.y) + V0.x;
        int Right = (UpperHalf ? 
            V2.x - (V2.x - V1.x) * (V2.y - y) / (V2.y - V1.y) : 
            V0.x + (V1.x - V0.x) * (y - V0.y) / (V1.y - V0.y));

        if (Left > Right) std::swap(Left, Right);

        for (int x = Left; x <= Right; x++)
          image.set(x, y, color);
    }
}

Vec3<int> gClearColor = Math::clampRGB(Vec3<float>(1.f, 0.98, 0.94) * 255);
// Vec3<int> gClearColor = Math::clampRGB(Vec3<float>(0.f, 0.f, 0.f) * 255);

// TODO: @ Rewrite whole rendering procedure
// TODO: @ Bulletproof .obj loading
// TODO: @ Benchmark

// TODO: @ Change to another .obj model
// TODO: @ SIMD 
// TODO: @ Speed up gamma correction by using gamma-lookup table
// TODO: @ Debug other scenes
// TODO: @ SSAO

// TODO: @ Other interesting post-processing techniques
// TODO: @ Debug view matrix(issues with camera)
// TODO: @ Anti-aliasing
// TODO: @ Optimization on rasterizing
// TODO: @ Raster top-left rule
int main(int argc, char* argv[]) {
    // ---------------------------
    glfwInit();
    Window window = { };
    window_manager.create_window(window, 800, 600);
    glfwMakeContextCurrent(window.m_window);
    glewInit();
    glClearColor(1.f, .5f, .4f, 1.f);

    window_manager.init_window(window);
    Scene scene;
    Renderer renderer;
    renderer.alloc_backbuffer(window);
    renderer.init();
    PhongShader phongShader;
    PBRShader pbrShader;
    DepthShader depthShader;
    phongShader.initFragmentAttrib(renderer.bufferWidth_, renderer.bufferHeight_);
    pbrShader.initFragmentAttrib(renderer.bufferWidth_, renderer.bufferHeight_);
    depthShader.initFragmentAttrib(renderer.bufferWidth_, renderer.bufferHeight_);
    renderer.shader_list.emplace_back(&phongShader);
    renderer.shader_list.emplace_back(&pbrShader);
    renderer.shader_list.emplace_back(&depthShader);

    // renderer.activeShaderPtr_ = &phongShader;
    renderer.activeShaderPtr_ = &pbrShader;
    // renderer.activeShaderPtr_ = &depthShader;
    SkyboxShader skyboxShader;
    renderer.skyboxShader_ = &skyboxShader;
    scene_manager.loadSceneFromFile(scene, "scenes/debug_scene/scene_config.json");

    // TODO: this should be move to part of Window class
    float quad[18] = {
        -1.f, 1.f, 0.f, 
         1.f,-1.f, 0.f, 
         1.f, 1.f, 0.f, 

        -1.f, 1.f, 0.f,
        -1.f,-1.f, 0.f,
         1.f,-1.f, 0.f,
    };

    float quad_uv[12] = {
        0.f, 1.f,
        1.f, 0.f,
        1.f, 1.f,

        0.f, 1.f,
        0.f, 0.f,
        1.f, 0.f
    };

    GLuint vertex_buffer, texture_uv_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &texture_uv_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, texture_uv_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_uv), quad_uv, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);

    glUseProgram(window.shader);
    renderer.clearBuffer(gClearColor);
    {
         PerformanceTimer renderTimer;
         renderer.drawScene(scene);
         window_manager.blit_buffer(renderer.backbuffer, renderer.bufferWidth_, renderer.bufferHeight_, 4, window);
    }

    while(!glfwWindowShouldClose(window.m_window)) {
        // Update transform of moving instances
        // scene_manager.updateScene(scene, 0.f);
        // Rendering
        glClear(GL_COLOR_BUFFER_BIT);
        renderer.clearBuffer(gClearColor); // clear color bit
        renderer.clearDepth();             // clear depth bit
        renderer.clearNormalBuffer();
        renderer.drawScene(scene);
        // renderer.SSAO();
        window_manager.blit_buffer(renderer.backbuffer, renderer.bufferWidth_, renderer.bufferHeight_, 4, window);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwPollEvents();
        glfwSwapBuffers(window.m_window);
    }

    return 0;
}   
