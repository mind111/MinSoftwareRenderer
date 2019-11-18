#pragma once

#include <vector>
#include <map>
#include "mesh.h"
#include "Math.h"

struct Camera {
    Vec3<float> position;
    Vec3<float> target;
    Vec3<float> world_up;

    float fov;
    float z_near;
    float z_far;
};

// TODO: Quaternion?
struct Transform {
    Vec3<float> translation;
    Vec3<float> rotation; // x, y, z
    Vec3<float> scale;
};

struct Texture {
    std::string textureName;
    std::string texturePath;
    int textureWidth, textureHeight, numChannels;
    unsigned char* pixels;
};

struct Light {
    Vec3<float> color;
    float intensity;
    virtual Vec3<float>* getPosition() = 0; 
    virtual Vec3<float>* getDirection() = 0;
};

struct DirectionalLight : Light {
    Vec3<float> direction;
    Vec3<float>* getPosition() override; 
    Vec3<float>* getDirection() override;
}; 

struct PointLight : Light {
    Vec3<float> position;
    float attentuation;
    Vec3<float>* getPosition() override; 
    Vec3<float>* getDirection() override;
};

struct Scene {
    int skyboxMeshID;
    std::vector<Mesh> mesh_list;
    std::vector<Transform> xform_list;
    std::vector<Mesh_Instance> instance_list;
    std::vector<Texture> texture_list;

    Camera main_camera;
    std::vector<PointLight> pointLightList;
    std::vector<DirectionalLight> directionalLightList;
};

class Scene_Manager {
public:
    Scene_Manager() {}
    void loadObj(Mesh& mesh, const char* filename);
    void loadSceneFromFile(Scene& scene, const char* filename);
    void add_instance(Scene& scene, uint32_t mesh_id);
    void loadTextureFromFile(Scene& scene, std::string& name, const char* filename);
    void findTextureForMesh(Scene& scene, Mesh& mesh);
    void findNormalMapForMesh(Scene& scene, Mesh& diablo_mesh);
    void updateScene(Scene& scene, float deltaTime);
    Mat4x4<float> get_camera_view(Camera& camera);
};

extern Scene_Manager scene_manager;