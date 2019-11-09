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

    void operator/(const T Scalar)
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
    static Vec3<float> barycentric(Vec2<float>* triangle, int x, int y, float denominator);
    static Vec3<float> bary_interpolate(Vec3<float>* vertices, const Vec3<float>& bary_coord); 
    static void bound_triangle(Vec2<float>* vertices, float* bounds);
    static Vec3<float> SampleAmbientDirection();
    static Vec3<float> CrossProduct(const Vec3<float>& V0, const Vec3<float>& V1);
    static Vec3<float> Normalize(const Vec3<float>& v);
    template <class T>
    static float Length(const Vec3<T>& V0);
    template <class T>
    inline static T DotProduct_Vec3(const Vec3<T>& V0, const Vec3<T>& V1)
    {
        return (V0.x * V1.x + V0.y * V1.y + V0.z * V1.z);
    }
    template <class T>
    inline static T DotProduct_Vec2(const Vec2<T>& V0, const Vec2<T>& V1)
    {
        return (V0.x * V1.x + V0.y * V1.y);
    }
};
