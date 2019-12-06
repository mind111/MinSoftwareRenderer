#include "mesh.h"
#include <iostream>
#include <fstream>
#include <sstream>

Mesh_Manager mesh_manager;

Mesh::Mesh() {
    vertex_buffer = nullptr;
    texture_uv_buffer = nullptr;
    normal_buffer = nullptr;
    tangentBuffer = nullptr;
    indices = nullptr;
    num_vertices = 0;
    num_texture_coord = 0;
    num_normal = 0;
    num_faces = 0;
    v_components = -1;
    vt_components = -1;
    vn_components = -1;
    idx_components = -1;
    normalMapID = -1;
    aoMapID = -1;
    roughnessMapID = -1;
}


Vec3<float> Mesh_Manager::getTangent(Mesh& mesh, uint32_t idx) {
    return Vec3<float>(mesh.tangentBuffer[idx * mesh.v_components],
                       mesh.tangentBuffer[idx * mesh.v_components + 1],
                       mesh.tangentBuffer[idx * mesh.v_components + 2]);
}
 
Vec3<float> Mesh_Manager::get_vertex(Mesh& mesh, uint32_t idx) {
    return Vec3<float>(mesh.vertex_buffer[idx * mesh.v_components], 
                       mesh.vertex_buffer[idx * mesh.v_components + 1], 
                       mesh.vertex_buffer[idx * mesh.v_components + 2]);
}

Vec3<float> Mesh_Manager::get_vt(Mesh& mesh, uint32_t idx) {
    return Vec3<float>(mesh.texture_uv_buffer[idx * mesh.vt_components], 
                       mesh.texture_uv_buffer[idx * mesh.vt_components + 1], 
                       mesh.texture_uv_buffer[idx * mesh.vt_components + 2]);
}

Vec3<float> Mesh_Manager::get_vn(Mesh& mesh, uint32_t idx) {
    return Vec3<float>(mesh.normal_buffer[idx * mesh.vn_components], 
                       mesh.normal_buffer[idx * mesh.vn_components + 1], 
                       mesh.normal_buffer[idx * mesh.vn_components + 2]);
}