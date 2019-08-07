#include <iostream>
#include "../Include/tgaimage.h"
#include "../Include/Math.h"

/// \TODO Clean up code to get rid of all the warnings

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

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

/// \TODO Simplify logic and optimization
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
        /// \Bug When slope is greater than 0, can invert x, y
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

/// \Note Using naive scan-line method
void FillTriangle(Vec2<int>& V0, Vec2<int>& V1, Vec2<int>& V2, TGAImage& image, const TGAColor& color)
{
    // Sort the vertices according to their y value
    if (V0.y > V1.y) V0.Swap(V1);
    if (V1.y > V2.y) V1.Swap(V2);

    /// \Note Compress code for rasterizing bottom half and upper half into one chunk
    for (int y = V0.y; y < V2.y; y++)
    {
        bool UpperHalf = (y >= V1.y);
        // Triangle similarity
        /// \Note Speed-up: extract the constant part of the formula, 
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

void RasterizeTriangle(Vec2<int> V0, Vec2<int> V1, Vec2<int> V2, TGAImage& image, TGAColor color)
{
    // Calculate the bounding box for the triangle
    int Bottom = V0.y, Up = V0.y, Left = V0.x, Right = V0.x;
    Vec2<int> E1 = V1 - V0;
    Vec2<int> E2 = V2 - V0;
    float Denom = E1.x * E2.y - E2.x * E1.y;
    Vec2<int> T[3];
    T[0] = V0;
    T[1] = V1;
    T[2] = V2;
    
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
            /// \Note Cramer's rule to solve for barycentric coordinates,
            ///       can also use ratio of area between three sub-triangles to solve
            ///       to solve for u,v,w
            Vec2<int> PA = V0 - Vec2<int>(x, y);
            float u = (-1 * PA.x * E2.y + PA.y * E2.x) / Denom;
            float v = (-1 * PA.y * E1.x + PA.x * E1.y) / Denom;
            float w = 1 - u - v;
            // Point p is not inside of the triangle
            if (u < 0 || v < 0 || w < 0 || u > 1 || v > 1 || w > 1)
                continue;
            
            image.set(x, y, color);
        }
    }

    return;
}

int main(int argc, char* argv[]) {
    // Create an image for writing pixels
    TGAImage image(200, 200, TGAImage::RGB);    

    /// \Note --- Triangles ---
    Vec2<int> V0(100, 50);
    Vec2<int> V1(150, 1);
    Vec2<int> V2(70, 180);

    Vec2<int> TriangleA[3];
    TriangleA[0] = Vec2<int>(10, 10);
    TriangleA[1] = Vec2<int>(10, 70);
    TriangleA[2] = Vec2<int>(60, 40);

    Vec2<int> TriangleB[3];
    TriangleB[0] = Vec2<int>(10, 20);
    TriangleB[1] = Vec2<int>(30, 120);
    TriangleB[2] = Vec2<int>(90, 40);
       
    DrawTriangle(TriangleA[0], TriangleA[1], TriangleA[2], image, TGAColor(40, 150, 100));
    DrawTriangle(TriangleB[0], TriangleB[1], TriangleB[2], image, white);
    DrawTriangle(V0, V1, V2, image, TGAColor(200, 50, 30, 255));

    RasterizeTriangle(TriangleA[0], TriangleA[1], TriangleA[2], image, TGAColor(40, 150, 100));
    RasterizeTriangle(TriangleB[0], TriangleB[1], TriangleB[2], image, TGAColor(40, 100, 200));
    RasterizeTriangle(V0, V1, V2, image, TGAColor(40, 150, 100));

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");

    return 0;
}   