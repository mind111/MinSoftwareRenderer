#pragma once
#include <string>
#include "Math.h"

struct Mesh_Instance {
    uint32_t mesh_id;
    uint32_t instance_id;
};

// TODO: Move texture names into this struct to consolidate Mesh class
struct MeshTextureParams {

};

class Mesh {
public:
    std::string name;
    std::string textureName;
    std::string normalMapName;
    float* vertex_buffer;
    float* texture_uv_buffer;
    float* normal_buffer;
    float* tangentBuffer;
    unsigned int* indices; // do not need index buffer if rearranging the vertex

    int num_vertices, num_faces, num_texture_coord, num_normal;
    int v_components, vt_components, vn_components, idx_components;
    int textureID, normalMapID;

    // TODO: These methods should be moved into Mesh_Manager
    Mesh();
    void load_obj(const char* filename);
    void load_texture(const char* filename);
};

class Mesh_Manager {
public:
    Vec3<float> get_vertex(Mesh& mesh, uint32_t idx);
    Vec3<float> get_vt(Mesh& mesh, uint32_t idx); 
    Vec3<float> get_vn(Mesh& mesh, uint32_t idx); 
    Vec3<float> getTangent(Mesh& mesh, uint32_t idx);
    void generateTangents(Mesh& mesh);
};


extern Mesh_Manager mesh_manager;