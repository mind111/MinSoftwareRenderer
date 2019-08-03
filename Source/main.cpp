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
        Vec2<T> Holder(0, 0);
        Holder = *this;
        *this = Another;
        Another = Holder;
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
    //**** Corner cases
    // Vertical line
    if (Start.x == End.x)
    {
        if (Start.y > End.y) Start.Swap(End);
        for (int i = Start.y; i <= End.y; i++)
        {
            image.set(Start.x, i, white);
        }

        return;
    }
    // Divide by 0
    if (Start.y == End.y)
    {
        if (Start.x > End.x) Start.Swap(End);
        for (int i = Start.x; i <= End.x; i++)
        {
            image.set(i, Start.y, white);
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
    if (Slope < 0) d *= -1;
    while (Next.x < End.x)
    {
        //**** Slope < 1
        if (std::abs(Slope) <= 1)
        {
            // Eval F(x+1,y+0.5)
            if (Next.y + d * 0.5f - Slope * (Next.x + 1.0f) - Intercept >= 0)
                Next = Next + Vec2<int>(1, 0);
            else
                Next = Next + Vec2<int>(1, d * 1);
        }
        /// \Bug When slope is greater than 0, can invert x, y
        else
        {
            if (Slope < 0)
            {
                // Eval F(x+0.5, y+1)
                if (Next.y + d * 1.0f - Slope * (Next.x + .5f) - Intercept >= 0)
                    Next = Next + Vec2<int>(0, d * 1);
                else
                    Next = Next + Vec2<int>(1, d * 1);
            }

            else
            {
                // Eval F(x+0.5, y+1)
                if (Next.y + d * 1.0f - Slope * (Next.x + .5f) - Intercept >= 0)
                    Next = Next + Vec2<int>(1, d * 1);
                else
                    Next = Next + Vec2<int>(0, d * 1);
            }

        }
        //**** Draw pixel to the buffer
        image.set((int)Next.x, (int)Next.y, color);
    }
}

/// \Note Assuming that the vertices are given in counter-clockwise order
void DrawTriangle(Vec2<int> V0, Vec2<int> V1, Vec2<int> V2, TGAImage& image, const TGAColor& color)
{
    DrawLine(V0, V1, image, color);
    DrawLine(V1, V2, image, color);
    DrawLine(V2, V0, image, color);
}

int main(int argc, char** argv) {
    // Create an image for writing pixels
    TGAImage image(200, 200, TGAImage::RGB);

    /// \Note --- Line Drawing ---
    /// \TODO Corner cases testing

    /// \Note --- Triangles ---
    Vec2<int> V0(10, 70);
    Vec2<int> V1(50, 160);
    Vec2<int> V2(70, 80);


    DrawTriangle(V0, V1, V2, image, TGAColor(200, 50, 30, 255));
    
    image.set(52, 41, red);
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");

    return 0;
}