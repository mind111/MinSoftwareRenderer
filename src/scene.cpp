#include "../include/scene.h"

Scene_Manager scene_manager;

Mat4x4<float> Scene_Manager::get_camera_view(Camera& camera) {
    Mat4x4<float> model_view;

    Vec3<float> forward = MathFunctionLibrary::Normalize(camera.target - camera.position);
    Vec3<float> right = MathFunctionLibrary::CrossProduct(forward, camera.world_up);   
    Vec3<float> up = MathFunctionLibrary::CrossProduct(right, forward);

    // --- Right ---
    // --- Up ------
    // --- Forward -
    model_view.Mat[0][0] = right.x;
    model_view.Mat[0][1] = right.y;
    model_view.Mat[0][2] = right.z;

    model_view.Mat[1][0] = up.x;
    model_view.Mat[1][1] = up.y;
    model_view.Mat[1][2] = up.z;

    model_view.Mat[2][0] = forward.x;
    model_view.Mat[2][1] = forward.y;
    model_view.Mat[2][2] = forward.z;

    model_view.Mat[3][0] = -camera.position.x;
    model_view.Mat[3][1] = -camera.position.y;
    model_view.Mat[3][2] = -camera.position.z;
    
    model_view.Mat[3][3] = 1;

    return model_view;
}