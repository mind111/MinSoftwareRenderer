#pragma once

#include "Math.h"

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

    Mat4x4<float> LookAt(Vec3<float> Direction)
    {
        Mat4x4<float> ModelView;

        // Update Camera's position in world accoridng to its translation
        this->Position += Translation;

        // Assuming that world up direction is always (0, 1, 0)
        this->Forward = MathFunctionLibrary::Normalize(Direction);
        // WorldUp is (0, 1, 0) by default
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
