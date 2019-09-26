#include <iostream>
#include "../Include/Math.h"

#define M_PI 3.14159265

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
