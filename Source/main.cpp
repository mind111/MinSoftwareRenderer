#include <iostream>
#include <fstream>
#include <string>
#include "../Include/tgaimage.h"
#include "../Include/Globals.h"
#include "../Include/Math.h"
#include "../Include/Model.h"

/// \TODO Clean up code to get rid of all the warnings
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Vec3<float> LightPos(0.0f, 0.5f, 1.0f);
Vec3<float> LightColor(0.7f, 0.7f, 0.7f);
Vec3<float> LightDir(0, 0, 1);

/// \TODO: Helper function for parsing wavefront .obj file
///        and profile this implementation against using STL
///        and stringstream
/// \TODO: Maybe turn this into a class and clean up the code

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

/// \TODO: Simplify logic and optimization
void DrawLine(Vec2<int> Start, Vec2<int> End, TGAImage& image, const TGAColor& color)
{
    int d = 1;
    //**** Divide by 0
    if (Start.x == End.x)
    {
        if (Start.y > End.y) Start.Swap(End);
        for (int i = Start.y; i <= End.y; i++)
        {
            image.set(Start.x, i, color);
        }

        return;
    }
    // Always start from left marching toward right
    // Swap Start and End
    if (Start.x > End.x) Start.Swap(End);
    // Derive the line function
    float Slope = (float)(Start.y - End.y) / (float)(Start.x - End.x);
    float Intercept = (float)End.y - Slope * End.x;
    Vec2<int> Next(Start);
    Vec2<int> StepA(1, 0);
    Vec2<int> StepB(1, 1);

    if (Slope < 0)
    {
        d *= -1;
        StepA.Swap(StepB);
    }
    if (Slope > 1 || Slope < -1)
    {
        StepA.Transpose();
        StepB.Transpose();
    }

    StepA.y *= d;
    StepB.y *= d;

    while (Next.x < End.x || Next.y != End.y)
    {
        //**** Slope < 1
        if (Slope >= -1 && Slope <= 1)
        {
            // Eval F(x+1,y+0.5)
            if (Next.y + d * 0.5f - Slope * (Next.x + 1.0f) - Intercept >= 0)
                Next += StepA;
            else
                Next += StepB;
        }
        /// \Bug: When slope is greater than 0, can invert x, y
        else
        {
            // Eval F(x+0.5, y+1)
            if (Next.y + d * 1.0f - Slope * (Next.x + .5f) - Intercept >= 0)
                Next += StepB;
            else
                Next += StepA;
        }
        //**** Draw pixel to the buffer
        image.set(Next.x, Next.y, color);
    }
}

void DrawTriangle(Vec2<int> V0, Vec2<int> V1, Vec2<int> V2, TGAImage& image, const TGAColor& color)
{
    DrawLine(V0, V1, image, color);
    DrawLine(V1, V2, image, color);
    DrawLine(V2, V0, image, color);
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

/** 
 * Given a point P on triangle's projection on screen space, use barycentric
 * coordinates to derive/interpolate P's actual position in world space.
 * 2D -> 3D. 
 * ** Orthographic case **
 * projection still preserves same barycentric coordinates as projected triangle
 * in 3D space
 * ** Perspective case **
 * TODO:
 */
bool UpdateDepthBuffer(Vec3<float> V0, Vec3<float> V1, Vec3<float> V2, 
                int ScreenX, int ScreenY, Vec3<float> Weights, float* ZBuffer)
{
    int Index = ScreenY * ImageWidth + ScreenX;
    float z = V0.z * Weights.x + V1.z * Weights.y + V2.z * Weights.z;

    if (z > ZBuffer[Index]) 
    {
        ZBuffer[Index] = z;
        return true;
    }

    return false;
}

Vec2<int> WorldToScreenOrtho(Vec3<float>& Vertex)
{
    return Vec2<int>((int)(Vertex.x * 400 + 400), (int)(Vertex.y * 400 + 400));
}

void RasterizeTriangle(Vec3<float> V0, Vec3<float> V1, Vec3<float> V2, 
        TGAImage& image, 
        Vec2<float>& V0_UV, 
        Vec2<float>& V1_UV, 
        Vec2<float>& V2_UV, 
        TGAImage* TextureImage,
        float* ZBuffer)
{
    // Project vertices of the triangle onto screen space using
    // orthographic projection
    Vec2<int> V0Screen = WorldToScreenOrtho(V0);
    Vec2<int> V1Screen = WorldToScreenOrtho(V1);
    Vec2<int> V2Screen = WorldToScreenOrtho(V2);

    // Calculate the bounding box for the triangle
    int Bottom = V0Screen.y, Up = V0Screen.y, Left = V0Screen.x, Right = V0Screen.x;
    Vec2<int> E1 = V1Screen - V0Screen;
    Vec2<int> E2 = V2Screen - V0Screen;
    float Denom = E1.x * E2.y - E2.x * E1.y;
    Vec2<int> T[3];
    T[0] = V0Screen;
    T[1] = V1Screen;
    T[2] = V2Screen;
    
    /// \TODO: I'm not sure if this is faster than calling std's min, max
    for (int i = 0; i < 3; i++)
    {
        if (T[i].x < Left) Left = T[i].x;
        if (T[i].x > Right) Right = T[i].x;
        if (T[i].y < Bottom) Bottom = T[i].y;
        if (T[i].y > Up) Up = T[i].y;
    }

    // Rasterization
    for (int x = Left; x <= Right; x++)
    {
        for (int y = Bottom; y <= Up; y++)
        {
            /// \Note: Cramer's rule to solve for barycentric coordinates,
            ///       can also use ratio of area between three sub-triangles to solve
            ///       to solve for u,v,w
            Vec2<int> PA = V0Screen - Vec2<int>(x, y);

            float u = (-1 * PA.x * E2.y + PA.y * E2.x) / Denom;
            float v = (-1 * PA.y * E1.x + PA.x * E1.y) / Denom;
            float w = 1 - u - v;

            Vec3<float> Weights(u, v, w);

            // Point p is not inside of the triangle
            if (u < 0 || v < 0 || w < 0 || u > 1 || v > 1 || w > 1)
                continue;

            // Depth test to see if current pixel is visible 
            if (UpdateDepthBuffer(V0, V1, V2, x, y, Weights, ZBuffer)) 
            {

                Vec2<float> MappedTexturePos(
                        Weights.z * V0_UV.x + Weights.x * V1_UV.x + Weights.y * V2_UV.x,
                        Weights.z * V0_UV.y + Weights.x * V1_UV.y + Weights.y * V2_UV.y
                );

                TGAColor Color = TextureImage->get(
                        TextureImage->get_width() * MappedTexturePos.x, 
                        TextureImage->get_height() * MappedTexturePos.y
                );

                image.set(x, y, TGAColor(
                            Color.bgra[2], 
                            Color.bgra[1], 
                            Color.bgra[0], 255
                ));
            }
        }
    }

    return;
}

///\TODO: Flat shading visuals still look a bit wierd, maybe it's an issue with surface
//        normal, need to further investigate
void DrawMesh(Graphx::Model& Model, TGAImage& image, TGAColor color, float* ZBuffer)
{
    Vec3<int>* IndexPtr = Model.Indices;
    int TriangleRendered = 0;

    while (TriangleRendered < 2492)
    {
        Vec2<int> V0 = WorldToScreenOrtho(Model.VertexBuffer[IndexPtr->x]);
        Vec2<int> V1 = WorldToScreenOrtho(Model.VertexBuffer[(IndexPtr + 1)->x]);
        Vec2<int> V2 = WorldToScreenOrtho(Model.VertexBuffer[(IndexPtr + 2)->x]);

        Vec3<float> V0V1 = Model.VertexBuffer[(IndexPtr + 1)->x] - Model.VertexBuffer[IndexPtr->x];
        Vec3<float> V0V2 = Model.VertexBuffer[(IndexPtr + 2)->x] - Model.VertexBuffer[IndexPtr->x];

        ///Note: Counter-clockwise vertex winding order
        Vec3<float> Normal = MathFunctionLibrary::Normalize(MathFunctionLibrary::CrossProduct(V0V1, V0V2));

	    float ShadingCoef = MathFunctionLibrary::DotProduct(LightDir, Normal);
        
        // ShadingCoef < 0 means that the triangle is facing away from the light, simply discard
        if (ShadingCoef < 0.0f) 
        {
            TriangleRendered++;
            IndexPtr += 3;
            continue;
        }
        
        if (ShadingCoef > 1.0f) ShadingCoef = 1.0f;
        
	    Vec3<float> Color = LightColor * ShadingCoef;

        // Hardcode to index 0 for now, need to add a ActiveTexture or something
        // Instead of fetch the color at vertices and interpolate them at each
        // overlapped pixel, interpolate uv coordinates and then use interpolated
        // uv to fetch color
        Vec2<float> V0_UV = Model.TextureBuffer[IndexPtr->y];
        Vec2<float> V1_UV = Model.TextureBuffer[(IndexPtr + 1)->y];
        Vec2<float> V2_UV = Model.TextureBuffer[(IndexPtr + 2)->y];

        RasterizeTriangle(Model.VertexBuffer[IndexPtr->x], 
                          Model.VertexBuffer[(IndexPtr + 1)->x], 
                          Model.VertexBuffer[(IndexPtr + 2)->x],
                          image, 
                          V0_UV,
                          V1_UV,
                          V2_UV, 
                          Model.TextureAssets[0],
                          ZBuffer);
        /// \Note: I ran into a gotcha here, I was using while(IndexPtr)
        ///         to dictate whether all the indices are traversed without
        ///         relizing that the memory right pass the last element in
        ///         Indices can as well be valid using memory using by something
        ///         else. Therefore, need to very careful with pointer arithmetic!!!
        IndexPtr += 3;
        TriangleRendered++;
    }
}

int main(int argc, char* argv[]) {
    int ImageSize = ImageWidth * ImageHeight;    
    // Create an image for writing pixels
    TGAImage image(ImageWidth, ImageHeight, TGAImage::RGB);
    char ModelPath[64] = { "../Graphx/Assets/Model.obj" };
    char TexturePath[64] = { "../Graphx/Assets/Textures/african_head_diffuse.tga" };

    // Add a scope here to help trigger Model's destructor
    {
        Graphx::Model Model;
        Model.Parse(ModelPath);
        TGAImage Sample;
        Model.LoadTexture(&Sample, TexturePath);
        Sample.flip_vertically();

        float* ZBuffer = new float[ImageWidth * ImageHeight];
        for (int i = 0; i < ImageSize; i++) ZBuffer[i] = -100.0f;

        // Draw the mesh
        ///\FIXME: This function call cause Heap corruption
        DrawMesh(Model, image, white, ZBuffer);
    }

    /// \TODO: Maybe instead of writing to an image,
    ///         can draw to a buffer, and display it using
    ///         a Win32 window
    // Draw the output to a file
    image.flip_vertically();
    image.write_tga_file("output.tga");

    return 0;
}   
