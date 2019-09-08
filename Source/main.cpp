#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
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
Vec3<float> CameraPos(0, 0, 3);

Mat4x4<float> ViewPort = Mat4x4<float>::ViewPort(ImageWidth, ImageHeight);

struct Camera
{
    Vec3<float> Position;
    Vec3<float> Translation;

    Vec3<float> Up;
    Vec3<float> Forward;
    Vec3<float> Right;

    Camera()
    {
        this->Position = Vec3<float>(0.f, 0.f, 0.f);
        this->Translation = Vec3<float>(0.f, 0.f, 0.f);
        this->Up = Vec3<float>(0.f, 1.f, 0.f);
        this->Forward = Vec3<float>(0.f, 0.f, -1.f);
        this->Right = Vec3<float>(1.f, 0.f, 0.f);
    }

    Mat4x4<float> LookAt(Vec3<float> Point)
    {
        Mat4x4<float> ModelView;

        // Update Camera's position in world accoridng to its translation
        this->Position += Translation;

        // Assuming that world up direction is always (0, 1, 0)
        this->Forward = MathFunctionLibrary::Normalize(Point - Position);
        this->Right = MathFunctionLibrary::Normalize(MathFunctionLibrary::CrossProduct(
                    this->Forward, Vec3<float>(0.f, 1.f, 0.f)));
        this->Up = MathFunctionLibrary::Normalize(MathFunctionLibrary::CrossProduct(
                    this->Right, this->Forward));

        // Construct the matrix using these three axes as basis
        // --- Right ---
        // --- Up ------
        // --- Forward -
        ModelView.Mat[0][0] = Right.x;
        ModelView.Mat[0][1] = Right.y;
        ModelView.Mat[0][2] = Right.z;

        ModelView.Mat[1][0] = Up.x;
        ModelView.Mat[1][1] = Up.y;
        ModelView.Mat[1][2] = Up.z;

        ModelView.Mat[2][0] = Forward.x;
        ModelView.Mat[2][1] = Forward.y;
        ModelView.Mat[2][2] = Forward.z;

        ModelView.Mat[3][3] = 1;

        Mat4x4<float> TransMatrix;
        TransMatrix.Identity();
        TransMatrix.Mat[0][3] = -1 * this->Translation.x;
        TransMatrix.Mat[1][3] = -1 * this->Translation.y;
        TransMatrix.Mat[2][3] = -1 * this->Translation.z; 
        
        return ModelView * TransMatrix; 
    }
};

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

Vec3<float> PerspectiveProjection(Vec3<float>& Vertex)
{
    // For now, default ZNear to z = 1
    float PerspectiveRatio = (CameraPos.z - 1) / (CameraPos.z - Vertex.z);
    // Perspective deform
    Vec3<float> CameraSpace_VertPos(Vertex.x * PerspectiveRatio,
                                    Vertex.y * PerspectiveRatio,
                                    Vertex.z);
    // Orthographic projection onto screen space
    return CameraSpace_VertPos;
}

///\TODO: Should not handle vertex transformation here, need to clean up
void RasterizeTriangle(Vec2<int> V0Screen, Vec2<int> V1Screen, Vec2<int> V2Screen, 
        Vec3<float> V0_World,
        Vec3<float> V1_World,
        Vec3<float> V2_World,
        TGAImage& image, 
        Vec2<float>& V0_UV, 
        Vec2<float>& V1_UV, 
        Vec2<float>& V2_UV, 
        TGAImage* TextureImage,
        float* ZBuffer)
{

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
            if (UpdateDepthBuffer(V0_World, V1_World, V2_World, x, y, Weights, ZBuffer)) 
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
    Mat4x4<float> ModelToWorld;
    ModelToWorld.Identity();
    // Translate the model further back
    ModelToWorld.SetTranslation(Vec3<float>(0.f, 0.f, -2.f));
    Mat4x4<float> Perspective = Mat4x4<float>::Perspective(-1.f, -10.f, 0.f);
    Camera LocalCamera;
    Mat4x4<float> View = LocalCamera.LookAt(Vec3<float>(0.f, 0.f, -1.f));

    ///\Note: Should not include 
    Mat4x4<float> MVP = Perspective 
                      //* View 
                      * ModelToWorld;
    //-- Debug --------
    ModelToWorld.Print();
    View.Print();
    ViewPort.Print();
    Perspective.Print();
    Mat4x4<float> Debug = View * ModelToWorld;
    MVP.Print();
    //-- Debug finish--
    
    while (TriangleRendered < 2492)
    {
        Vec3<float> V0_Vec3 = Model.VertexBuffer[IndexPtr->x];
        Vec3<float> V1_Vec3 = Model.VertexBuffer[(IndexPtr + 1)->x];
        Vec3<float> V2_Vec3 = Model.VertexBuffer[(IndexPtr + 2)->x];

        Vec4<float> V0(V0_Vec3.x, V0_Vec3.y, V0_Vec3.z, 1.0f);
        Vec4<float> V1(V1_Vec3.x, V1_Vec3.y, V1_Vec3.z, 1.0f);
        Vec4<float> V2(V2_Vec3.x, V2_Vec3.y, V2_Vec3.z, 1.0f);

        // Model->World->Camera->Clip
        Vec4<float> V0Clip_Vec4 = MVP * V0;
        Vec4<float> V1Clip_Vec4 = MVP * V1;
        Vec4<float> V2Clip_Vec4 = MVP * V2;
        
        // Need to divde (w-component - 1) of transformed points
        V0Clip_Vec4.x = V0Clip_Vec4.x / (V0Clip_Vec4.w - 1);
        V0Clip_Vec4.y = V0Clip_Vec4.y / (V0Clip_Vec4.w - 1);
        V1Clip_Vec4.x = V1Clip_Vec4.x / (V1Clip_Vec4.w - 1);
        V1Clip_Vec4.y = V1Clip_Vec4.y / (V1Clip_Vec4.w - 1);
        V2Clip_Vec4.x = V2Clip_Vec4.x / (V2Clip_Vec4.w - 1);
        V2Clip_Vec4.y = V2Clip_Vec4.y / (V2Clip_Vec4.w - 1);
        
        // Resetting the w back to 1 or else would mess up the computation
        // for Viewport transformation
        V0Clip_Vec4.w = 1.f;
        V1Clip_Vec4.w = 1.f;
        V2Clip_Vec4.w = 1.f;

        // Then apply viewport transformation
        Vec4<float> V0Screen_Vec4 = ViewPort * V0Clip_Vec4;
        Vec4<float> V1Screen_Vec4 = ViewPort * V1Clip_Vec4;
        Vec4<float> V2Screen_Vec4 = ViewPort * V2Clip_Vec4;
        
        Vec2<int> V0Screen(std::ceil(V0Screen_Vec4.x), std::ceil(V0Screen_Vec4.y));
        Vec2<int> V1Screen(std::ceil(V1Screen_Vec4.x), std::ceil(V1Screen_Vec4.y));
        Vec2<int> V2Screen(std::ceil(V2Screen_Vec4.x), std::ceil(V2Screen_Vec4.y));

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

        RasterizeTriangle(V0Screen, 
                          V1Screen, 
                          V2Screen,
                          Model.VertexBuffer[IndexPtr->x],
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
    // Testing Matrix multiplication
    Camera Camera;

    int ImageSize = ImageWidth * ImageHeight;    
    // Create an image for writing pixels
    TGAImage image(ImageWidth, ImageHeight, TGAImage::RGB);
    char ModelPath[64] = { "../Graphx/Assets/Model.obj" };
    char TexturePath[64] = { "../Graphx/Assets/Textures/african_head_diffuse.tga" };

    Mat4x4<float> ViewMat;
    ViewMat.Print();
    Camera.Position = CameraPos;
    Camera.Translation = Vec3<float>(0.0, 0.0, 0.0);
    ViewMat.Identity();
    ViewMat = Camera.LookAt(Vec3<float>(0.0f, 1.0f, -1.0f));
    ViewMat.Print();

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
