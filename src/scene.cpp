#include "../include/scene.h"
#include "../lib/stb_image/include/stb_image.h"

Scene_Manager scene_manager;

Vec3<float>* DirectionalLight::getPosition() {
    return nullptr;
}

Vec3<float>* DirectionalLight::getDirection() {
    return &direction;
}

Vec3<float>* PointLight::getPosition() {
    return &position;
}

Vec3<float>* PointLight::getDirection() {
    return nullptr;
}

Mat4x4<float> Scene_Manager::get_camera_view(Camera& camera) {
    Mat4x4<float> model_view;

    Vec3<float> forward = Math::Normalize(camera.position - camera.target);
    Vec3<float> right = Math::CrossProduct(forward, camera.world_up);   
    Vec3<float> up = Math::CrossProduct(right, forward);

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

void Scene_Manager::loadTextureFromFile(Scene& scene, std::string& name, const char* filename) {
    Texture newTexture = { };
    newTexture.textureName = name;
    newTexture.texturePath = filename;
    newTexture.pixels = stbi_load(filename, &newTexture.textureWidth, &newTexture.textureHeight, &newTexture.numChannels, 0);
    scene.texture_list.emplace_back(newTexture);
}

void Scene_Manager::findTextureForMesh(Scene& scene, Mesh& mesh) {
    for (int i = 0; i < scene.texture_list.size(); i++) {
        if (scene.texture_list[i].textureName == mesh.textureName) {
            mesh.textureID = i;
        }
    }
}