#pragma once
#include <cmath>
#include <algorithm>
#include <random>

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

    Vec2<T> operator*(T scalar) {
        return Vec2<T>(this->x * scalar, this->y * scalar);
    }

    Vec2<T> operator=(const Vec2<T>& Another)
    {
        this->x = Another.x;
        this->y = Another.y;
        return *this;
    }
    
    void operator*=(const T Scalar)
    {
        this->x *= Scalar;
        this->y *= Scalar;
    }

    Vec2<T> operator/(const T Scalar)
    {
	return Vec2<T>(this.x / Scalar, this.y / Scalar);
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

template <class T>
struct Vec3
{
    T x;
    T y;
    T z;

    //**** Constructor
    Vec3() {

    }

    Vec3(T p, T q, T r)
    {
        x = p;
        y = q;
        z = r;
    }
    
    Vec3(T s) {
        x = s;
        y = s;
        z = s;
    }

    Vec3(const Vec3<T>& Another)
    {
        this->x = Another.x;
        this->y = Another.y;
        this->z = Another.z;
    }

    //**** Operations
    Vec3<T> operator-(const Vec3<T>& Another)
    {
        return Vec3(this->x - Another.x, this->y - Another.y, this->z - Another.z);
    }

    Vec3<T> operator+(const Vec3<T>& Another)
    {
        return Vec3<T>(this->x + Another.x, this->y + Another.y, this->z + Another.z);
    }
   
    Vec3<T> operator*(T Scalar)
    {
        return Vec3<T>(this->x * Scalar, this->y * Scalar, this->z * Scalar);        
    }

    Vec3<T> operator/(T Scalar)
    {
      return Vec3<T>(this->x / Scalar, this->y / Scalar, this->z / Scalar);
    }

    Vec3<T> operator*(Vec3<T>& rhs) {
        Vec3<T> res;
        res.x = this->x * rhs.x;
        res.y = this->y * rhs.y;
        res.z = this->z * rhs.z;
        return res;
    }

    Vec3<T>& operator/=(T scalar) {
        this->x /= scalar;
        this->y /= scalar;
        this->z /= scalar;
        return *this;
    }
  
    Vec3<T> operator=(const Vec3<T>& Another)
    {
        this->x = Another.x;
        this->y = Another.y;
        this->z = Another.z;
        return *this;
    }

    Vec3<T>& operator+=(const Vec3<T>& Another)
    {
        this->x += Another.x;
        this->y += Another.y;
        this->z += Another.z;
        return *this;
    }

    Vec3<T>& operator*=(const T scalar) {
        this->x *= scalar;
        this->y *= scalar;
        this->z *= scalar;
        return *this;
    }

    void Swap(Vec3<T>& Another)
    {
        Vec3<T> Holder;
        Holder = *this;
        *this = Another;
        Another = Holder;
    }

    void Print();
};

template <class T>
struct Vec4
{
    T x;
    T y;
    T z;
    T w;

    Vec4() {}
    Vec4(T x, T y, T z, T w): x(x), y(y), z(z), w(w) {}
    Vec4(Vec3<T> V, T w): x(V.x), y(V.y), z(V.z), w(w) {}

    Vec4<T> operator+(const Vec4& rhs) {
        Vec4<T> result;
        result.x = this->x + rhs.x; 
        result.y = this->y + rhs.y; 
        result.z = this->z + rhs.z; 
        return result;
    }

    Vec4<T>& operator+=(const Vec4& rhs) {
        this->x += rhs.x;
        this->y += rhs.y;
        this->z += rhs.z;
        this->w += rhs.w;
        return *this;
    }

    void operator=(const Vec4& RHS)
    {
        this->x = RHS.x;
        this->y = RHS.y;
        this->z = RHS.z;
        this->w = RHS.w;
    }

    void operator-=(const Vec4& RHS)
    {
        this->x -= RHS.x;
        this->y -= RHS.y;
        this->z -= RHS.z;
        this->w -= RHS.w;
    }

    Vec4<T>& operator/(const T scalar) {
        this->x /= scalar;
        this->y /= scalar;
        this->z /= scalar;
        this->w /= scalar;
        return *this;
    }

    Vec4<T>& operator*(const T scalar) {
        this->x *= scalar;
        this->y *= scalar;
        this->z *= scalar;
        this->w *= scalar;
        return *this;
    }

    void Print();

};

template <class T>
struct Mat2x2
{
    T Mat[2][2];

    Mat2x2()    
    {
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 2; j++)
                Mat[i][j] = 0;
        }

    }
    
    Vec4<T> operator*()
    {

    }

    Mat2x2<T> Inverse()
    {
        Mat2x2<float> Result;
        float Determinant = Mat[0][0] * Mat[1][1] - Mat[0][1] * Mat[1][0]; 
        Result.Mat[0][0] =  Mat[1][1] / Determinant;
        Result.Mat[0][1] = -Mat[0][1] / Determinant;
        Result.Mat[1][0] = -Mat[1][0] / Determinant;
        Result.Mat[1][1] =  Mat[0][0] / Determinant;
        return Result;
    }
};

template <class T>
struct Mat4x4
{
    T Mat[4][4];

    Mat4x4()    
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
                Mat[i][j] = 0;
        }

    }
    
    Vec4<T> operator*(const Vec4<T>& v)
    {
        Vec4<T> Res;

        Res.x = Mat[0][0] * v.x + Mat[0][1] * v.y + 
                Mat[0][2] * v.z + Mat[0][3] * v.w;

        Res.y = Mat[1][0] * v.x + Mat[1][1] * v.y + 
                Mat[1][2] * v.z + Mat[1][3] * v.w;

        Res.z = Mat[2][0] * v.x + Mat[2][1] * v.y + 
                Mat[2][2] * v.z + Mat[2][3] * v.w;

        Res.w = Mat[3][0] * v.x + Mat[3][1] * v.y + 
                Mat[3][2] * v.z + Mat[3][3] * v.w;

        return Res;
    }

    Mat4x4<T> operator*(const Mat4x4<T>& v)
    {
        Mat4x4<T> Res;

        for (int i = 0 ; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                int Row = i;
                int Col = j;

                for (int k = 0; k < 4; k++)
                    Res.Mat[i][j] += this->Mat[Row][k] * v.Mat[k][Col];
            }
        }

        return Res;
    }
    
    void operator=(const Mat4x4& Other)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
                this->Mat[i][j] = Other.Mat[i][j];
        }
    }

    void SetRow(int RowIndex, Vec4<T> v);
    void SetColumn(int ColIndex, Vec4<T> v);
    void SetTranslation(Vec3<float> v);
    void SetRotation(Vec3<float> r);
    Mat4x4<float> Inverse();
    void Identity()
    {
        this->Mat[0][0] = 1;
        this->Mat[1][1] = 1;
        this->Mat[2][2] = 1;
        this->Mat[3][3] = 1;
    }


    /**
     * ---------- Utils ----------
     *
     */
    void Print();
    static Mat4x4<float> viewport(float VP_Width, float VP_Height);

    // Projection matrix should operate on camera frame
    static Mat4x4<float> Perspective(float aspectRatio, float near, float far, float fov);
};

class Math
{
public:
    static float clamp_f(float x, float min, float max);
    static void clampVec3f(Vec3<float>& v, float minValue, float maxValue);
    static Vec3<int> clampRGB(Vec3<float> color);
    static Vec3<float> reflect(Vec3<float>& v, Vec3<float> normal);
    static Vec3<float> barycentric(Vec3<float>* triangle, float x, float y, float denominator);
    static Vec3<float> bary_interpolate(Vec3<float>* vertices, const Vec3<float>& bary_coord); 
    static void boundTriangle(Vec3<float>* vertices, float* bounds, float bufferWidth, float bufferHeight);
    static Vec3<float> SampleAmbientDirection();
    static Vec3<float> CrossProduct(const Vec3<float>& V0, const Vec3<float>& V1);
    static Mat4x4<float> constructTransformMatrix(const struct Transform& t);
    // transform
    static void translate(Mat4x4<float>& m, Vec3<float> translation);
    static void rotate(Mat4x4<float>& m, Vec3<float> rotAxis, float degree);
    static void scale(Mat4x4<float>& m, Vec3<float> scaleVector);
    template <class T>
    static float length(const Vec3<T>& V0);
    template <class T>
    inline static T dotProductVec3(const Vec3<T>& V0, const Vec3<T>& V1)
    {
        return (V0.x * V1.x + V0.y * V1.y + V0.z * V1.z);
    }
    template <class T>
    inline static T DotProduct_Vec2(const Vec2<T>& V0, const Vec2<T>& V1)
    {
        return (V0.x * V1.x + V0.y * V1.y);
    }

    inline static Vec3<float> normalize(const Vec3<float>& v) {
        float len = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        return Vec3<float>(v.x / len, v.y / len, v.z / len);
    }
};

#define PI 3.1415926