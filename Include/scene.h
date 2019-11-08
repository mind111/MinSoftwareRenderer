#pragma once

#include "mesh.h"
#include "Math.h"
#include <vector>

struct Camera {
    Vec3<float> position;
    Vec3<float> target;
    Vec3<float> world_up;

    float fov;
    float z_near;
    float z_far;
};

struct Texture {
    unsigned char* pixels;
};

struct Light {
    Vec3<float> color;
};

struct DirectionalLight : Light {
    Vec3<float> direction;
}; 

struct Scene {
    std::vector<Mesh> mesh_list;
    //std::vector<Material> material_list;
    std::vector<Mat4x4<float>> xform_list;
    std::vector<Mesh_Instance> instance_list;

    Camera main_camera;
    std::vector<Light> light_list;
};

class Scene_Manager {
public:
    Scene_Manager() {}
    void load_scene_form_file(const char* filename);
    void add_instance(Scene& scene, uint32_t mesh_id);
    void loadTextureFromFile(const char* filename);
    Mat4x4<float> get_camera_view(Camera& camera);
};

extern Scene_Manager scene_manager;