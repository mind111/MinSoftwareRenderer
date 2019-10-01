#include <iostream>
#include "../Include/Math.h"

#define M_PI 3.14159265

// Using basic elementary row operations to derive inverse matrix
Mat4x4<float> Mat4x4<float>::Inverse()
{
    Mat4x4<float> Res;
    float Res_Augmented[4][8];

    // Augment M with elementary matrix on the right
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
            Res_Augmented[i][j] = this->Mat[i][j];

        for (int j = 4; j < 8; j++)
            Res_Augmented[i][j] = (float)(i == (j - 4));
    }
    
    // First pass to elminate the bottom left half of the matrix to 0
    for (int i = 0; i < 4; i++)
    {
        // Normalize diagnal entry on current row to 1
        for (int j = 7; j >= 0; j--)
           Res_Augmented[i][j] /= Res_Augmented[i][i]; 

        // Use the normalized row to eliminate Res[Row][0] to 0
        for (int Row = i + 1; Row < 4; Row++)
        {
            float Coef = Res_Augmented[Row][i];
            for (int Col = 0; Col < 8; Col++)
                Res_Augmented[Row][Col] -= Coef * Res_Augmented[i][Col];
        }
    }

    // Second pass to elminate the Upper Right half of the matrix to 0
    for (int i = 3; i >= 0; i--)
    {
        // Use the normalized row to eliminate Res[Row][0] to 0
        for (int Row = i - 1; Row >= 0; Row--)
        {
            float Coef = Res_Augmented[Row][i];
            for (int Col = Row; Col < 8; Col++)
                Res_Augmented[Row][Col] -= Coef * Res_Augmented[i][Col];
        }
    }

    // Get the right half of the augmented M
    for (int i = 0; i < 4; i++)
    {
        for (int j = 4; j < 8; j++) 
            Res.Mat[i][j - 4] = Res_Augmented[i][j];
    }

    return Res;
}

void Mat4x4<float>::SetRow(int RowIndex, Vec4<float> v)
{
    Mat[RowIndex][0] = v.x;
    Mat[RowIndex][1] = v.y;
    Mat[RowIndex][2] = v.z;
    Mat[RowIndex][3] = v.w;
}

void Mat4x4<float>::SetColumn(int ColIndex, Vec4<float> v)
{
    Mat[0][ColIndex] = v.x;
    Mat[1][ColIndex] = v.y;
    Mat[2][ColIndex] = v.z;
    Mat[3][ColIndex] = v.w;
}

void Mat4x4<float>::SetTranslation(Vec3<float> v)
{
    this->Mat[0][3] = v.x;
    this->Mat[1][3] = v.y;
    this->Mat[2][3] = v.z;
}

void Mat4x4<float>::SetRotation(Vec3<float> r) 
{

}

void Vec3<float>::Print()
{
    std::cout << "[ " << this->x << " "
                      << this->y << " "
                      << this->z << " ]" << std::endl;
}

void Vec4<float>::Print()
{
    std::cout << "[ " << this->x << " "
                      << this->y << " "
                      << this->z << " "
                      << this->w << " ]" << std::endl;
}

void Mat4x4<float>::Print()
{
    std::cout << "-- Mat4x4 --" << std::endl;
    for (int Row = 0; Row < 4; Row++)
    {
        std::cout << "[ ";
        for (int Col = 0; Col < 4; Col++)
           std::cout << this->Mat[Row][Col] << " "; 
        std::cout << "]\n";
    }
}

// TODO: Need to handle remapping of Z from world space to [0, 1] 
Mat4x4<float> Mat4x4<float>::Perspective(float AspectRatio, 
                                         float Near, 
                                         float Far, 
                                         float FOV)
{ 
    Mat4x4<float> Res;
    float r = std::tan((FOV / 2) * M_PI / 180.f); // Converting degrees to radians
    float Left = Near * r * AspectRatio;
    float Right = -Left;
    float Bottom = Near * r;
    float Up = -Bottom;

    Res.Identity();
    
    // ---------------------
    // [ 1, 0, 0,         0]
    // [ 0, 1, 0,         0]
    // [ 0, 0, 1,         0]
    // [ 0, 0, -1 / Near, 1]
    // ---------------------
    Res.Mat[3][2] = -1.f / Near;

    // Handle conversion from world space to NDC space
    Res.Mat[0][0] = 2.f / (Right - Left); 
    Res.Mat[0][2] = -(Right + Left) / (Right - Left);

    // Maybe need to consider aspect-ratio here
    Res.Mat[1][1] = 2.f / (Up - Bottom); 
    Res.Mat[1][2] = -(Up + Bottom) / (Up - Bottom);

    return Res;
}

Mat4x4<float> Mat4x4<float>::ViewPort(float VP_Width, float VP_Height)
{
   // Map [-1, 1] to [0, VP_Width]
   
   Mat4x4<float> Res;

   Res.Identity();
   Res.Mat[0][0] = .5f * VP_Width;
   Res.Mat[0][3] = .5f * VP_Width;
   Res.Mat[1][1] = .5f * VP_Height;
   Res.Mat[1][3] = .5f * VP_Height;

   return Res;
}

float MathFunctionLibrary::clamp_f(float x, float min, float max)
{
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

void MathFunctionLibrary::bound_triangle(Vec2<float>* vertices, float* bounds)
{
    for (int i = 0; i < 3; i++)
    {
        if (vertices[i].x < bounds[0]) bounds[0] = vertices[i].x;
        if (vertices[i].x > bounds[1]) bounds[1] = vertices[i].x;
        if (vertices[i].y < bounds[2]) bounds[2] = vertices[i].y;
        if (vertices[i].y > bounds[3]) bounds[3] = vertices[i].y;
    }

    for (int i = 0; i < 3; i++)
        MathFunctionLibrary::clamp_f(bounds[i], 0.f, 799.f);
}

// TODO: Fix this so that it samples uniformly from the surface
//       of a hemi-sphere
Vec3<float> MathFunctionLibrary::SampleAmbientDirection()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribution(-1.f, 1.f);
    float x = 1.f, y = 1.f, z = 1.f;

    x = distribution(gen);
    y = distribution(gen);
    z = distribution(gen);

    return MathFunctionLibrary::Normalize(Vec3<float>(x, y, z));
}

template <class T>
inline float MathFunctionLibrary::Length(const Vec3<T>& V0)
{
    return sqrt(V0.x * V0.x + V0.y * V0.y + V0.z * V0.z);
}

Vec3<float> MathFunctionLibrary::CrossProduct(const Vec3<float>& V0, const Vec3<float>& V1)
{
    /// \Note: |i     -j     k|
    ///        |v0.x v0.y v0.z|
    ///        |v1.x v1.y v1.z|
    Vec3<float> Result;

    Result.x = V0.y * V1.z - V0.z * V1.y;
    Result.y = V0.z * V1.x - V0.x * V1.z;
    Result.z = V0.x * V1.y - V0.y * V1.x;

    return Result;
}

Vec3<float> MathFunctionLibrary::Normalize(const Vec3<float>& v)
{
    float Len = Length(v);
    return Vec3<float>(v.x / Len, v.y / Len, v.z / Len);
}
