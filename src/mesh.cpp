#include "../include/mesh.h"
#include <iostream>
#include <fstream>
#include <sstream>

Mesh_Manager mesh_manager;

Mesh::Mesh() {
    vertex_buffer = nullptr;
    texture_uv_buffer = nullptr;
    normal_buffer = nullptr;
    indices = nullptr;
    num_vertices = 0;
    num_texture_coord = 0;
    num_normal = 0;
    num_faces = 0;
    v_components = -1;
    vt_components = -1;
    vn_components = -1;
    idx_components = -1;
    textureID = -1;
    normalMapID = -1;
}

void Mesh::load_texture(const char* filename) {

}

// Helpers for parsing obj
int split_by_spaces(std::string& s) {
    int result = 0;
    for (int i = 0; i < s.length(); i++) {
        if (s[i] != ' ') {
            result++;                    
            while(s[i] != '\0' && s[i] != ' ') {
                i++;
            }
        }
    }
    return result;
}

// Helpers for parsing obj
// TODO: This won't work for .obj that has format like 1//3, it will still produce 3 components for each index
int count_slash(std::string& s) {
    int result = 0;
    for (int i = 0; i < s.length(); i++) {
        if (s[i] == '/') {
            result++;                    
        }
    }
    return result;
}

// Helpers for parsing obj
int get_index(std::string& s) {
    int result = 0;
    int l = 0, r = 0;

    for ( ; r < s.length(); r++) {
        if (s[r] == '/') {
            break;
        }
    }
    result = std::stoi(s.substr(l, r - l));
    s = s.substr(s.length() == r ? 0 : r + 1);
    return result;
}

void processIndexString(std::string& indexStr, unsigned int* indexArray, int numComponents) {
    std::string token, dump;
    std::stringstream ss(indexStr);
    ss >> dump;
    while (ss >> token) {
        for (int i = 0; i < numComponents; i++) {
            *(indexArray + i) = get_index(token);
        }
        indexArray += 3;
    } 
}

// TODO: @ Add a parameter "stride"
Vec3<float> fetchPositionFromBuffer(const float* buffer, int idx) {
    return Vec3<float>(buffer[idx * 3], buffer[idx * 3 + 1],
                       buffer[idx * 3 + 2]);
}

Vec2<float> fetchTextureUVFromBuffer(const float* buffer, int idx, int numComponents, int offset) {
    return Vec2<float>(buffer[idx * numComponents + offset], buffer[idx * numComponents + offset + 1]);
}

Vec3<float> computeTangent(Vec3<float>& v0, Vec3<float>& v1, Vec3<float>& v2,
                           Vec2<float>& uv0, Vec2<float>& uv1, Vec2<float>& uv2) {
    Vec3<float> tangent;
    Mat2x2<float> a;
    Vec3<float> e1 = v1 - v0;
    Vec3<float> e2 = v2 - v0;
    Vec2<float> deltaUV1 = uv1 - uv0;
    Vec2<float> deltaUV2 = uv2 - uv0;
    a.Mat[0][0] = deltaUV1.x; // U1 - U0 
    a.Mat[0][1] = deltaUV1.y; // V1 - V0
    a.Mat[1][0] = deltaUV2.x; // U2 - U0
    a.Mat[1][1] = deltaUV2.y; // V2 - V0
    Mat2x2<float> aInv = a.Inverse(); 
    tangent.x = aInv.Mat[0][0] * e1.x + aInv.Mat[0][1] * e2.x;
    tangent.y = aInv.Mat[0][0] * e1.y + aInv.Mat[0][1] * e2.y;
    tangent.z = aInv.Mat[0][0] * e1.z + aInv.Mat[0][1] * e2.z;
    return Math::Normalize(tangent);
}

// TODO: @ Maybe compute tangent at load time
// TODO: @ Maybe benchmark this?
void Mesh::load_obj(const char* filename) {
    using std::string;
    using std::stringstream;

    std::ifstream file(filename);
    if (!file.is_open()) std::cerr << "Cannot open file: " << filename << std::endl;

    string line, dump, token;
    string delim = "/";

    int index_sub_component = -1;
    
    // first pass
    while (std::getline(file, line)) {
        stringstream ss(line);
        if (line[0] == 'v' && line[1] == ' ') {
            if (v_components == -1) {
                v_components = split_by_spaces(line) - 1;
            }
            this->num_vertices++;
        } else if (line[0] == 'v' && line[1] == 't') {
            if (vt_components == -1) {
                vt_components = split_by_spaces(line) - 1;
                texture_uv_buffer = new float [vt_components * 3 * num_faces];
            }
            num_texture_coord++;
        } else if (line[0] == 'v' && line[1] == 'n') {
            if (vn_components == -1) {
                vn_components = split_by_spaces(line) - 1;
                normal_buffer = new float [vn_components * 3 * num_faces];
            }
            num_normal++;
        } else if (line[0] == 'f') {
            if (idx_components == -1) {
                idx_components = split_by_spaces(line) - 1;
                ss >> dump;
                ss >> token;
                index_sub_component = count_slash(token) + 1;
            }
            this->num_faces++;
        }
    }

    file.clear();
    file.seekg(0);

    // allocate memory 
    float* vertex_buffer_raw = new float[num_vertices * v_components];
    float* texture_uv_buffer_raw = 0; 
    float* normal_buffer_raw = 0;
    float* tangentBufferRaw = 0;

    if (vt_components != -1) {
        texture_uv_buffer_raw = new float[num_texture_coord * vt_components];
        tangentBufferRaw = new float[num_vertices * v_components];
        for (int i = 0; i < num_vertices * v_components; i++) {
            tangentBufferRaw[i] = 0.f;
        }
        tangentBuffer = new float[3 * num_faces * v_components];
        texture_uv_buffer = new float[3 * num_faces * vt_components];
    }
    if (vn_components != -1) {
        normal_buffer_raw = new float[num_normal * vn_components];
        normal_buffer = new float[3 * num_faces * vn_components];
    } 

    vertex_buffer = new float[3 * num_faces * v_components];
    indices = new unsigned int[3 * num_faces];

    int loaded_vertex = 0, loaded_vt = 0, loaded_vn = 0, loaded_index = 0, loadedFace = 0;
    Vec3<float> p0, p1, p2;
    Vec2<float> uv0, uv1, uv2;
    
    // second pass
    while (std::getline(file, line)) {
        // Parse the line
        stringstream ss(line);
        
        if (line.find("v ") != string::npos) {
            ss >> dump; // dump the line prefix like v, vt, vn, f
            for (int i = 0; i < v_components; i++) {
                ss >> vertex_buffer_raw[loaded_vertex * v_components + i];  
            }
            loaded_vertex++;
        } else if (line[0] == 'v' && line[1] == 't') {
            ss >> dump;
            for (int i = 0; i < vt_components; i++) {
                ss >> texture_uv_buffer_raw[loaded_vt * vt_components + i];
            }
            loaded_vt++;
        } else if (line[0] == 'v' && line[1] == 'n') {
            ss >> dump;
            for (int i = 0; i < vt_components; i++) {
                ss >> normal_buffer_raw[loaded_vn * vn_components + i];
            }
            loaded_vn++;
        } else if (line[0] == 'f') {
            unsigned int faceIndexArray[9] = {0};
            processIndexString(line, faceIndexArray, index_sub_component);
            // generate tangent
            if (index_sub_component > 1) {
                p0 = fetchPositionFromBuffer(vertex_buffer_raw, faceIndexArray[0] - 1);
                p1 = fetchPositionFromBuffer(vertex_buffer_raw, faceIndexArray[3] - 1);
                p2 = fetchPositionFromBuffer(vertex_buffer_raw, faceIndexArray[6] - 1);
                uv0 = fetchTextureUVFromBuffer(texture_uv_buffer_raw, faceIndexArray[1] - 1, vt_components, 0);
                uv1 = fetchTextureUVFromBuffer(texture_uv_buffer_raw, faceIndexArray[4] - 1, vt_components, 0);
                uv2 = fetchTextureUVFromBuffer(texture_uv_buffer_raw, faceIndexArray[7] - 1, vt_components, 0);
                // compute tangents
                Vec3<float> faceTangent = computeTangent(p0, p1, p2, uv0, uv1, uv2);
                for (int i = 0; i < 3; i++) {
                    int tangentIndex = (faceIndexArray[i * 3] - 1) * 3; 
                    tangentBufferRaw[tangentIndex]     += faceTangent.x;
                    tangentBufferRaw[tangentIndex + 1] += faceTangent.y;
                    tangentBufferRaw[tangentIndex + 2] += faceTangent.z;
                }
            }
            ss >> dump;
            // TODO: the idx_components here seems misleading, it actually indicate
            // how many indices per face
            for (int i = 0; i < idx_components; i++) {
                unsigned int index;
                string token;
                unsigned int loaded_sub_component = 1;
                ss >> token;
                index = get_index(token);
                int nextVertex = loaded_index * v_components; 
                vertex_buffer[nextVertex] = vertex_buffer_raw[(index - 1) * v_components];  
                vertex_buffer[nextVertex + 1] = vertex_buffer_raw[(index - 1) * v_components + 1];  
                vertex_buffer[nextVertex + 2] = vertex_buffer_raw[(index - 1) * v_components + 2];  
                indices[loaded_index] = index;
            
                for ( ; loaded_sub_component < index_sub_component; loaded_sub_component++) {
                    switch (loaded_sub_component % index_sub_component) {
                        case 1: {
                            index = get_index(token);
                            texture_uv_buffer[loaded_index * vt_components] = texture_uv_buffer_raw[(index - 1) * vt_components];
                            texture_uv_buffer[loaded_index * vt_components + 1] = texture_uv_buffer_raw[(index - 1) * vt_components + 1];
                            texture_uv_buffer[loaded_index * vt_components + 2] = texture_uv_buffer_raw[(index - 1) * vt_components + 2];
                        } break;

                        case 2: {
                            index = get_index(token);
                            normal_buffer[loaded_index * vn_components] = normal_buffer_raw[(index - 1) * vn_components];
                            normal_buffer[loaded_index * vn_components + 1] = normal_buffer_raw[(index - 1) * vn_components + 1];
                            normal_buffer[loaded_index * vn_components + 2] = normal_buffer_raw[(index - 1) * vn_components + 2];
                        } break;

                        default: {
                        }
                    }
                }
                loaded_index++;
            }
        }
    }

    if (tangentBufferRaw) {
        for (int i = 0; i < num_vertices; i++) {
            Vec3<float> summedTangents(tangentBufferRaw[i * v_components],
                                       tangentBufferRaw[i * v_components + 1],
                                       tangentBufferRaw[i * v_components + 2]);
            Vec3<float> normalizedTangent = Math::Normalize(summedTangents);
            tangentBufferRaw[i * v_components] = normalizedTangent.x;
            tangentBufferRaw[i * v_components + 1] = normalizedTangent.y;
            tangentBufferRaw[i * v_components + 2] = normalizedTangent.z;
        }
        // reorder the tangents to match vertex buffer
        for (int i = 0; i < num_faces * 3; i++) {
            int vertexIdx = (indices[i] - 1) * 3;
            tangentBuffer[i * 3] = tangentBufferRaw[vertexIdx];
            tangentBuffer[i * 3 + 1] = tangentBufferRaw[vertexIdx + 1];
            tangentBuffer[i * 3 + 2] = tangentBufferRaw[vertexIdx + 2];
        }
    }

    // Clean up
    delete []vertex_buffer_raw;
    delete []texture_uv_buffer_raw;
    delete []normal_buffer_raw;
    delete []tangentBufferRaw;
    delete []indices;
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