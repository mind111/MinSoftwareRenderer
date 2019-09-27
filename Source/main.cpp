#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <algorithm>
#include "../Include/tgaimage.h"
#include "../Include/Globals.h"
#include "../Include/Math.h"
#include "../Include/Model.h"
#include "../Include/Camera.h"
#include "../Include/Shader.h"

/// \TODO Clean up code to get rid of all the warnings
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Vec3<float> LightPos(0.0f, 0.5f, 1.0f);
Vec3<float> LightColor(0.7f, 0.7f, 0.7f);
Vec3<float> LightDir(0, 0, 1);
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
    // Testing Matrix multiplication
    Camera Camera;
    int ImageSize = ImageWidth * ImageHeight;    
    // Create an image for writing pixels
    TGAImage image(ImageWidth, ImageHeight, TGAImage::RGB);
    TGAImage ShadowImage(ImageWidth, ImageHeight, TGAImage::RGB);

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
    Camera.Position = CameraPos;
    Camera.Translation = Vec3<float>(0.0, 0.0, 0.0);
    Mat4x4<float> View = Camera.LookAt(Vec3<float>(0.0f, 0.0f, -1.0f));

    // Projection
    Mat4x4<float> Perspective = Mat4x4<float>::Perspective(1.f, -1.f, -5.f, 90.f);
    
    Shader Shader;
    Shader.VS.Model = ModelToWorld;
    Shader.VS.MVP = Perspective * View * ModelToWorld;
    Shader.VS.Viewport = Mat4x4<float>::ViewPort(ImageWidth, ImageHeight);
    
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

        float* ZBuffer = new float[ImageWidth * ImageHeight];
        float* ShadowBuffer = new float[ImageWidth * ImageHeight];
        for (int i = 0; i < ImageSize; i++) ZBuffer[i] = -100.0f;
        for (int i = 0; i < ImageSize; i++) ShadowBuffer[i] = -100.0f;

        Shader.FS.ZBuffer = ZBuffer;
        Shader.FS.ShadowBuffer = ShadowBuffer;
        Shader.DrawShadow(Model, ShadowImage, LightPos, LightDir, ShadowBuffer);
        Shader.Draw(Model, image, Camera, Shader_Mode::Phong_Shader);
    }

    /// \TODO: Maybe instead of writing to an image,
    ///         can draw to a buffer, and display it using
    ///         a Win32 window
    // Draw the output to a file
    image.flip_vertically();
    image.write_tga_file("output.tga");

    return 0;
}   
