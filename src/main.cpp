#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include "tgaimage.h"
#include "Globals.h"
#include "Model.h"
#include "window.h"
#include "scene.h"
#include "renderer.h"
#include "Shader.h"

/// \TODO Clean up code to get rid of all the warnings
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Vec3<float> LightPos(3.0f, 0.5f, -2.0f);
Vec3<float> LightColor(0.7f, 0.7f, 0.7f);
Vec3<float> LightDir(1.f, 0, 0.f); // @Simple directional_light
Vec3<float> CameraPos(0, 0, 1);

/// \Note More optimized version of DrawLine, Inspired by GitHub ssloy/tinyrenderer
void Line(Vec2<int> Start, Vec2<int> End, TGAImage& image, const TGAColor& color)
{
    bool Steep = false;
    int d = 1;
    if (Start.x > End.x) Start.Swap(End);
    if (Start.y > End.y) d = -1;
    // Slope < 1
    if (std::abs(Start.x - End.x) > std::abs(Start.y - End.y))
    {
        Steep = true;
        for (int i = Start.x; i <= End.x; i++)
        {
            float Ratio = (i - Start.x) / (End.x - Start.x);
            int y = Start.y + d * Ratio * (End.y - Start.y);
        }
    }
    else
    {
        
    }
}

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

/*
// @ Leave this out for now
void generate_occlusion_texture(Model& Model, Shader& shader) {
        int number_of_ab_samples = 1;
        int image_size = ImageWidth * ImageHeight;    
        TGAImage occlusion_texture(1024, 1024, TGAImage::RGB);
        Camera occlusion_camera;
        Shader occlusion_shader;
        occlusion_shader.VS.Model = shader.VS.Model;
        occlusion_shader.VS.Projection = shader.VS.Projection;
        occlusion_shader.VS.Viewport = shader.VS.Viewport;
        TGAImage* occlusion_samples = new TGAImage[1];
        for (int i = 0; i < number_of_ab_samples; i++)
            occlusion_samples[i] = TGAImage(1024, 1024, TGAImage::RGB);
        float* occlusion_depth_buffer =  new float[ImageWidth * ImageHeight];

        for (int i = 0; i < number_of_ab_samples; i++) 
        {
            // Flush the buffer
            for (int i = 0; i < image_size; i++) occlusion_depth_buffer[i] = 100.0f;
            // Random generate a ambient light direction
            Vec3<float> ab_light_direction = Math::SampleAmbientDirection();

            // Vec3<float>(0.f, 0.f, -2.f) here refers to the center of the model
        //    occlusion_camera.Translation = Vec3<float>(0.f, 0.f, -2.f) + ab_light_direction * 2.f;
         //   Mat4x4<float> occlusion_view = occlusion_camera.LookAt(ab_light_direction);
          //  occlusion_shader.VS.MVP = occlusion_shader.VS.Projection * occlusion_view * occlusion_shader.VS.Model;
            occlusion_shader.DrawOcclusion(Model, occlusion_samples[i], occlusion_depth_buffer);
        }

        for (int x = 0; x < 1024; x++)
        {
            for (int y = 0; y < 1024; y++)
            {
                Vec3<float> avg_color(0.f, 0.f, 0.f);

                for (int i = 0; i < number_of_ab_samples; i++) 
                {
                    TGAColor sample_color = occlusion_samples[i].get(x, y);
                    avg_color.x += sample_color[2]; // R
                    avg_color.y += sample_color[1]; // G
                    avg_color.z += sample_color[0]; // B
                }

                avg_color.x /= number_of_ab_samples;
                avg_color.y /= number_of_ab_samples;
                avg_color.z /= number_of_ab_samples;
                occlusion_texture.set(x, y, TGAColor(avg_color.x, avg_color.y, avg_color.z));
            }
        }

        occlusion_texture.flip_vertically();
        occlusion_texture.write_tga_file("occlusion_texture.tga");
}
*/

// TODO: @ Rewrite whole rendering procedure
// TODO: @ Bulletproof .obj loading
// TODO: @ Benchmark

// TODO: @ Change to another .obj model
// TODO: @ Skybox 
//       @ Maybe use openGL to handle texture sampling for skybox
//       @
// TODO: @ SSAO
// TODO: @ become real time, requires multi-threading & SIMD
// TODO: @ For some reasons, normal mapping is not working, DEBUG!!
int main(int argc, char* argv[]) {
    // TODO: @ render the renderer's backbuffer to a texture
    // TODO: @ Using the renderer's backbuffer as texture data
    // TODO: @ and then render the texture to the screen
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
    Phong_Shader phongShader;
    phongShader.initFragmentAttrib(renderer.buffer_width, renderer.buffer_height);
    renderer.shader_list.emplace_back(&phongShader);
    renderer.activeShaderPtr_ = &phongShader;
    SkyboxShader skyboxShader;
    renderer.skyboxShader_ = &skyboxShader;
    scene_manager.loadSceneFromFile(scene, "scenes/default_scene/scene_config.json");

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
    renderer.clearBuffer();
    {
         PerformanceTimer renderTimer;
         renderer.drawScene(scene);
    }

    while(!glfwWindowShouldClose(window.m_window)) {
        // Update transform of moving instances
        //scene_manager.updateScene(scene, 0.f);
        // Rendering
        glClear(GL_COLOR_BUFFER_BIT);
        renderer.clearBuffer(); // clear color bit
        renderer.clearDepth();  // clear depth bit
        renderer.drawScene(scene);
        window_manager.blit_buffer(renderer.backbuffer, renderer.buffer_width, renderer.buffer_height, 4, window);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwPollEvents();
        glfwSwapBuffers(window.m_window);
    }

    return 0;
}   
