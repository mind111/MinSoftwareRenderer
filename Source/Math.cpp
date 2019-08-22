#include "../Include/Math.h"
 
template <class T>
float MathFunctionLibrary::Length(const Vec3<T>& V0)
{
    return sqrt(V0.x * V0.x + V0.y * V0.y + V0.z * V0.z);
}

Vec3<float> MathFunctionLibrary::CrossProduct(const Vec3<float>& V0, const Vec3<float>& V1)
{
    /// \Note: |i     j      k|
    ///        |v0.x v0.y v0.z|
    ///        |v1.x v1.y v0.z|
    Vec3<float> Result;

    Result.x = V0.y * V0.z - V0.z * V1.y;
    Result.y = V0.z * V1.x - V0.x * V0.z;
    Result.z = V0.x * V1.y - V0.y * V1.x;

    return Result;
}

Vec3<float> MathFunctionLibrary::Normalize(const Vec3<float>& v)
{
    float Len = Length(v);
    return Vec3<float>(v.x / Len, v.y / Len, v.z / Len);
}
