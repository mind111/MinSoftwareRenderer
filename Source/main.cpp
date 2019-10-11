#include <iostream>
#include <string>
#include <cmath>
#include "../Include/tgaimage.h"
#include "../Include/Globals.h"
#include "../Include/Model.h"
#include "../Include/Camera.h"
#include "../Include/Shader.h"

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

int main(int argc, char* argv[]) {
    Camera camera;
    int image_size = ImageWidth * ImageHeight;    
    int number_of_ab_samples = 1;
    // Create an image for writing pixels
    TGAImage occlusion_texture(1024, 1024, TGAImage::RGB);
    TGAImage image(ImageWidth, ImageHeight, TGAImage::RGB);
    TGAImage shadow_image(ImageWidth, ImageHeight, TGAImage::RGB);

    // Mesh .obj file path
    char ModelPath[64] = { "../Graphx/Assets/Mesh/Model.obj" };
    char ModelPath_Diablo[64] = { "../Graphx/Assets/Mesh/diablo3_pose.obj" };
    char TexturePath[64] = { "../Graphx/Assets/Textures/african_head_diffuse.tga" };
    char TexturePath_Diablo[64] = { "../Graphx/Assets/Textures/diablo3_pose_diffuse.tga"};
    char NormalPath[64] = { "../Graphx/Assets/Textures/african_head_nm_tangent.tga" };
    char NormalPath_Diablo[64] = { "../Graphx/Assets/Textures/diablo3_pose_nm_tangent.tga" };

    // Model
    Mat4x4<float> ModelToWorld;
    ModelToWorld.Identity();
    ModelToWorld.SetTranslation(Vec3<float>(0.f, 0.f, -2.f));
    // View
    camera.Position = CameraPos;
    camera.Translation = Vec3<float>(0.0, 0.0, 0.0);
    Mat4x4<float> View = camera.LookAt(Vec3<float>(0.0f, 0.0f, -1.0f));

    // Projection
    Mat4x4<float> Perspective = Mat4x4<float>::Perspective(1.f, -1.f, -5.f, 90.f);
    
    Shader shader;
    shader.VS.Model = ModelToWorld;
    shader.VS.Projection = Perspective;
    shader.VS.MVP = Perspective * View * ModelToWorld;
    shader.VS.Viewport = Mat4x4<float>::ViewPort(ImageWidth, ImageHeight);
    
    // Add a scope here to help trigger Model's destructor
    {
        Model Model;
        Model.Parse(ModelPath_Diablo);
        TGAImage Texture;
        TGAImage NormalTexture;
        Model.LoadTexture(&Texture, TexturePath_Diablo);
        Texture.flip_vertically();
        Model.LoadNormalMap(&NormalTexture, NormalPath_Diablo);
        NormalTexture.flip_vertically();
        TGAImage* occlusion_samples = new TGAImage[1];
        for (int i = 0; i < number_of_ab_samples; i++)
            occlusion_samples[i] = TGAImage(1024, 1024, TGAImage::RGB);
        float* occlusion_depth_buffer =  new float[ImageWidth * ImageHeight];
        float* ZBuffer = new float[ImageWidth * ImageHeight];
        float* ShadowBuffer = new float[ImageWidth * ImageHeight];
        for (int i = 0; i < image_size; i++) ZBuffer[i] = 100.0f;
        for (int i = 0; i < image_size; i++) ShadowBuffer[i] = 100.0f;
        shader.FS.ZBuffer = ZBuffer;
        shader.FS.ShadowBuffer = ShadowBuffer;

        Camera occlusion_camera;
        Shader occlusion_shader;
        occlusion_shader.VS.Model = shader.VS.Model;
        occlusion_shader.VS.Projection = shader.VS.Projection;
        occlusion_shader.VS.Viewport = shader.VS.Viewport;

        for (int i = 0; i < number_of_ab_samples; i++) 
        {
            // Flush the buffer
            for (int i = 0; i < image_size; i++) occlusion_depth_buffer[i] = 100.0f;
            // Random generate a ambient light direction
            Vec3<float> ab_light_direction = MathFunctionLibrary::SampleAmbientDirection();

            occlusion_camera.Translation = Vec3<float>(0.f, 0.f, -2.f) + ab_light_direction * 2.f;
            Mat4x4<float> occlusion_view = occlusion_camera.LookAt(ab_light_direction);
            occlusion_shader.VS.MVP = occlusion_shader.VS.Projection * occlusion_view * occlusion_shader.VS.Model;
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

        Model.LoadTexture(&Texture, "occlusion_texture.tga");
        Texture.flip_vertically();
        //Shader.DrawShadow(Model, shadow_image, LightPos, LightDir, ShadowBuffer);
        shader.Draw(Model, image, camera, Shader_Mode::Phong_Shader);
    }

    /// \TODO: Maybe instead of writing to an image,
    ///         can draw to a buffer, and display it using
    ///         a Win32 window
    // Draw the output to a file
    image.flip_vertically();
    image.write_tga_file("output.tga");
    shadow_image.flip_vertically();
    shadow_image.write_tga_file("shadow.tga");

    return 0;
}   
