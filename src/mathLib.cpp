#include <iostream>
#include "scene.h"
#include "mathLib.h"

// Using basic elementary row operations to derive inverse matrix 
template <>
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

template <>
void Mat4x4<float>::SetRow(int RowIndex, Vec4<float> v)
{
    Mat[RowIndex][0] = v.x;
    Mat[RowIndex][1] = v.y;
    Mat[RowIndex][2] = v.z;
    Mat[RowIndex][3] = v.w;
}

template <>
void Mat4x4<float>::SetColumn(int ColIndex, Vec4<float> v)
{
    Mat[0][ColIndex] = v.x;
    Mat[1][ColIndex] = v.y;
    Mat[2][ColIndex] = v.z;
    Mat[3][ColIndex] = v.w;
}

template <>
void Mat4x4<float>::SetTranslation(Vec3<float> v)
{
    this->Mat[0][3] = v.x;
    this->Mat[1][3] = v.y;
    this->Mat[2][3] = v.z;
}

template <>
// r denotes degrees in x, y, z
void Mat4x4<float>::SetRotation(Vec3<float> r) {
    
}

template <>
void Vec3<float>::Print()
{
    std::cout << "[ " << this->x << " "
                      << this->y << " "
                      << this->z << " ]" << std::endl;
}

template <>
void Vec4<float>::Print()
{
    std::cout << "[ " << this->x << " "
                      << this->y << " "
                      << this->z << " "
                      << this->w << " ]" << std::endl;
}

template <>
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
// TODO: @Find out whether zNear and zFar and defined in world space or view space
//        to fix the sign issues within this function
// TODO: @For now assuming that zNear and zFar are defined in view space (all positive)
template <>
Mat4x4<float> Mat4x4<float>::Perspective(float aspectRatio, 
                                         float near, 
                                         float far, 
                                         float fov)
{
    Mat4x4<float> res;
    float r = std::tan((fov / 2) * PI / 180.f); // Converting degrees to radians
    float top = near * r;
    float bottom = -top;
    float left = bottom * aspectRatio;
    float right = -left;

    res.Identity();
    res.Mat[3][3] = 0.f;
    
    // negative 1 since after the view transformation, the z should all be positive for
    // all the vertices in the scene, while zNear and zFar are defined in world coordinate
    // space which are negative
    res.Mat[3][2] = -1.f; // make the result of multiplication has -z in w-component

    // Handle conversion from world space to NDC space
    // x is mapped to [-1, 1]
    // y is mapped to [-1, 1]
    // z is mapped to [0, 1]
    res.Mat[0][0] = 2.f * near / (right - left); 
    res.Mat[0][2] = (right + left) / (right - left);
    res.Mat[1][1] = 2.f * near / (top - bottom); 
    res.Mat[1][2] = (top + bottom) / (top - bottom);
    res.Mat[2][2] = -far / (far - near);
    res.Mat[2][3] = -far * near / (far - near);

    return res;
}

template <>
Mat4x4<float> Mat4x4<float>::viewport(float VP_Width, float VP_Height) {
   // Map [-1, 1] to [0, VP_Width]
   Mat4x4<float> Res;

   Res.Identity();
   Res.Mat[0][0] = .5f * VP_Width;
   Res.Mat[0][3] = .5f * VP_Width;
   Res.Mat[1][1] = .5f * VP_Height;
   Res.Mat[1][3] = .5f * VP_Height;

   return Res;
}

float Math::clamp_f(float x, float min, float max)
{
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

void Math::clampVec3f(Vec3<float>& v, float minValue, float maxValue) {
    if (v.x > maxValue) v.x = maxValue;
    if (v.x < minValue) v.x = minValue;
    if (v.y > maxValue) v.y = maxValue;
    if (v.y < minValue) v.y = minValue;
    if (v.z > maxValue) v.z = maxValue;
    if (v.z < minValue) v.z = minValue;
}

// TODO: Do I need to do std::ceil(here) ?
Vec3<int> Math::clampRGB(Vec3<float> color) {
    Vec3<int> result(color.x, color.y, color.z);
    if (result.x > 255) result.x = 255;
    if (result.y > 255) result.y = 255;
    if (result.z > 255) result.z = 255;
    return result; 
}

Vec3<float> Math::barycentric(Vec3<float>* triangle, float x, float y, float denominator) {
    Vec2<float> vScreen0(triangle[0].x, triangle[0].y);
    Vec2<float> pa = vScreen0 - Vec2<float>(x, y);
    Vec2<float> e1 = Vec2<float>(triangle[1].x, triangle[1].y) - vScreen0;
    Vec2<float> e2 = Vec2<float>(triangle[2].x, triangle[2].y) - vScreen0;
    float u = (-1 * pa.x * e2.y + pa.y * e2.x) / denominator;
    float v = (-1 * pa.y * e1.x + pa.x * e1.y) / denominator;
    float w = 1 - u - v;
    return Vec3<float>(w, u, v);
}

Vec3<float> Math::bary_interpolate(Vec3<float>* vertices, const Vec3<float>& bary_coord) {
    Vec3<float> result;
    result = vertices[0] * bary_coord.x + vertices[1] * bary_coord.y + vertices[2] * bary_coord.z;
    return result;
}

// TODO: Bug here
void Math::boundTriangle(Vec3<float>* vertices, float* bounds, float bufferWidth, float bufferHeight)
{
    for (int i = 0; i < 3; i++)
    {
        if (vertices[i].x < bounds[0]) bounds[0] = vertices[i].x;
        if (vertices[i].x > bounds[1]) bounds[1] = vertices[i].x;
        if (vertices[i].y < bounds[2]) bounds[2] = vertices[i].y;
        if (vertices[i].y > bounds[3]) bounds[3] = vertices[i].y;
    }

    for (int i = 0; i < 2; i++)
        bounds[i] = Math::clamp_f(bounds[i], 0.f, bufferWidth);
    for (int i = 2; i < 4; i++)
        bounds[i] = Math::clamp_f(bounds[i], 0.f, bufferHeight);
}

// TODO: Fix this so that it samples uniformly from the surface
//       of a hemi-sphere
Vec3<float> Math::SampleAmbientDirection()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribution(0.f, .5f);

    float u = distribution(gen);
    float v = distribution(gen);
    float theta = 2.f * PI * u;
    float phi = std::acos(2.f * v - 1.f);

    float x = std::cos(theta) * std::sin(phi);
    float y = std::abs(std::sin(theta) * std::sin(phi));
    float z = std::cos(theta);

    return Vec3<float>(x, y, z);
}

template <class T>
inline float Math::length(const Vec3<T>& V0)
{
    return sqrt(V0.x * V0.x + V0.y * V0.y + V0.z * V0.z);
}

Vec3<float> Math::CrossProduct(const Vec3<float>& V0, const Vec3<float>& V1)
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

// reflected ray should be automatically normalized
Vec3<float> Math::reflect(Vec3<float>& v, Vec3<float> normal) {
    Vec3<float> result = normal * 2 * Math::dotProductVec3(normal, v) - v;
    return result;
}

void Math::translate(Mat4x4<float>& m, const Vec3<float> v) {
    Mat4x4<float> t;
    t.Identity();
    t.SetColumn(3, Vec4<float>(v, 1.f));
    m = t * m;
}

void Math::rotate(Mat4x4<float>& m, const Vec3<float> axis, float degree) {

}

void Math::scale(Mat4x4<float>& m, const Vec3<float> s) {
    Mat4x4<float> t;
    t.Identity();
    t.Mat[0][0] *= s.x;
    t.Mat[1][1] *= s.y;
    t.Mat[2][2] *= s.z;
    m = t * m;
}

Mat4x4<float> Math::constructTransformMatrix(const Transform& t) {
    // pitch <- yaw, not dealing with roll for now
    Mat4x4<float> rotYaw, rotPitch, rotation;
    Mat4x4<float> m;
    m.Identity();
    // TODO: I'm still confused about how handed-ness affect sign of rotation
    rotYaw.Identity();
    rotYaw.Mat[0][0] = std::cos(t.rotation.x);
    rotYaw.Mat[0][2] = std::sin(t.rotation.x);
    rotYaw.Mat[2][0] = -std::sin(t.rotation.x);
    rotYaw.Mat[2][2] = std::cos(t.rotation.x);

    rotPitch.Identity();
    rotPitch.Mat[1][1] = std::cos(t.rotation.y);
    rotPitch.Mat[1][2] = -std::sin(t.rotation.y);
    rotPitch.Mat[2][1] = std::sin(t.rotation.y);
    rotPitch.Mat[2][2] = std::cos(t.rotation.y);
    rotation = rotPitch * rotYaw;

    // worth noting that the order of transformation is
    // reversed compared to the order in well-know glm
    // since I'm using row-major matrix here
    Math::scale(m, t.scale);
    m = rotation * m; // use this for rotation for now, impl general rotation later
    Math::translate(m, t.translation);
    return m;
}
