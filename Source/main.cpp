#include <cmath>
#include <algorithm>
#include <iostream>
#include "../Include/tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

template <class T>
struct Vec2
{
    T x;
    T y;

    //**** Constructor
    Vec2()
    {
        
    }

    Vec2(T p, T q) 
    {
        x = p;
        y = q;
    }

    Vec2(const Vec2<T>& Another)
    {
        this->x = Another.x;
        this->y = Another.y;
    }

    //**** Operations
    Vec2<T> operator-(const Vec2<T>& Another)
    {
        return Vec2(this->x - Another.x, this->y - Another.y);
    }

    Vec2<T> operator+(const Vec2<T>& Another)
    {
        return Vec2<T>(this->x + Another.x, this->y + Another.y);
    }

    Vec2<T> operator=(const Vec2<T>& Another)
    {
        this->x = Another.x;
        this->y = Another.y;
        return *this;
    }

    Vec2<T>& operator+=(const Vec2<T>& Another)
    {
        this->x += Another.x;
        this->y += Another.y;

        return *this;
    }

    void Swap(Vec2<T>& Another)
    {
        Vec2<T> Holder;
        Holder = *this;
        *this = Another;
        Another = Holder;
    }

    void Transpose()
    {
        T Holder;
        Holder = this->x;
        this->x = this->y;
        this->y = Holder;
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

/// \Note Assuming that the vertices are given in counter-clockwise order
void DrawTriangle(Vec2<int> V0, Vec2<int> V1, Vec2<int> V2, TGAImage& image, const TGAColor& color)
{
    DrawLine(V0, V1, image, color);
    DrawLine(V1, V2, image, color);
    DrawLine(V2, V0, image, color);
}

void RasterizeTriangle(Vec2<int>& V0, Vec2<int>& V1, Vec2<int>& V2, TGAImage& image, const TGAColor& color)
{
    // Sort the vertices according to their y value
    if (V0.y > V1.y) V0.Swap(V1);
    if (V1.y > V2.y) V1.Swap(V2);

    // Rasterize the bottem half,
    for (int y = V0.y; y < V1.y; y++)
    {
        int Left, Right;

        // Triangle similarity
        Left = (V2.x - V0.x) * (y - V0.y) / (V2.y - V0.y) + V0.x;
        Right = (V1.x - V0.x) * (y - V0.y) / (V1.y - V0.y) + V0.x;

        if (Left > Right) std::swap(Left, Right);

        for (int x = Left; x < Right; x++)
            image.set(x, y, color);
    }

    /// \TODO Cleanup and test for both cases when V2.x > V0.x and V2.x < V0.x
    // Rasterize the upper half;
    for (int y = V1.y; y < V2.y; y++)
    {
        int Left, Right;

        //if (V2.x > V0.x)
        //{ 
            Left = V0.x + (V2.x - V0.x) * (y - V0.y) / (V2.y - V0.y);
            Right = V2.x - (V2.x - V1.x) * (V2.y - y) / (V2.y - V1.y);
        //}

        if (Left > Right) std::swap(Left, Right);

        for (int x = Left; x < Right; x++)
            image.set(x, y, color);
    }
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

    RasterizeTriangle(V0, V1, V2, image, TGAColor(40, 150, 100));
    RasterizeTriangle(TriangleA[0], TriangleA[1], TriangleA[2], image, TGAColor(40, 150, 100));
    RasterizeTriangle(TriangleB[0], TriangleB[1], TriangleB[2], image, TGAColor(40, 100, 200));

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");

    return 0;
}   