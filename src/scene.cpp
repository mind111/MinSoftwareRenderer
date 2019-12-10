#include <iostream>
#include <fstream>
#include <sstream>
#include "../include/scene.h"
#include "../lib/stb_image/include/stb_image.h"
#include "../lib/json/json.hpp"

SceneManager scene_manager;

//---- Utilities for loading the scene data from json
void from_json(const nlohmann::json& j, Vec3<float>& v) { 
    v.x = j.at(0);
    v.y = j.at(1);
    v.z = j.at(2);
}

// TODO: handle rotation and scale
void from_json(const nlohmann::json& j, Mat4x4<float>& m) {
    Vec3<float> translation = j.at("translation").get<Vec3<float>>();
    m.Identity();
    m.SetTranslation(translation);
}

void from_json(const nlohmann::json& j, Transform& t) {
    t.translation = j.at("translation").get<Vec3<float>>();
    t.rotation = j.at("rotation").get<Vec3<float>>();
    t.scale = j.at("scale").get<Vec3<float>>();
}

void from_json(const nlohmann::json& j, Camera& c) {
    c.position = j.at("position").get<Vec3<float>>();
    c.target = j.at("target").get<Vec3<float>>();
    c.world_up = j.at("world_up").get<Vec3<float>>();
    j.at("fov").get_to(c.fov);
    j.at("z_far").get_to(c.z_far);
    j.at("z_near").get_to(c.z_near);
}

void from_json(const nlohmann::json& j, Mesh& mesh) {
    j.at("name").get_to(mesh.name);
}
//-------------------------------------------------------

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

Mat4x4<float> SceneManager::get_camera_view(Camera& camera) {
    Mat4x4<float> model_view;

    Vec3<float> forward = Math::normalize(camera.position - camera.target);
    Vec3<float> right = Math::CrossProduct(camera.world_up, forward);   
    Vec3<float> up = Math::CrossProduct(forward, right);

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
    return Math::normalize(tangent);
}

// TODO: @ Maybe benchmark this?
void SceneManager::loadObj(Mesh& mesh, const char* filename) {
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
            if (mesh.v_components == -1) {
                mesh.v_components = split_by_spaces(line) - 1;
            }
            mesh.num_vertices++;
        } else if (line[0] == 'v' && line[1] == 't') {
            if (mesh.vt_components == -1) {
                mesh.vt_components = split_by_spaces(line) - 1;
                mesh.texture_uv_buffer = new float [mesh.vt_components * 3 * mesh.num_faces];
            }
            mesh.num_texture_coord++;
        } else if (line[0] == 'v' && line[1] == 'n') {
            if (mesh.vn_components == -1) {
                mesh.vn_components = split_by_spaces(line) - 1;
                mesh.normal_buffer = new float [mesh.vn_components * 3 * mesh.num_faces];
            }
            mesh.num_normal++;
        } else if (line[0] == 'f') {
            if (mesh.idx_components == -1) {
                mesh.idx_components = split_by_spaces(line) - 1;
                ss >> dump;
                ss >> token;
                index_sub_component = count_slash(token) + 1;
            }
            mesh.num_faces++;
        }
    }

    file.clear();
    file.seekg(0);

    // allocate memory 
    float* vertex_buffer_raw = new float[mesh.num_vertices * mesh.v_components];
    float* texture_uv_buffer_raw = 0; 
    float* normal_buffer_raw = 0;
    float* tangentBufferRaw = 0;

    if (mesh.vt_components != -1) {
        texture_uv_buffer_raw = new float[mesh.num_texture_coord * mesh.vt_components];
        tangentBufferRaw = new float[mesh.num_vertices * mesh.v_components];
        for (int i = 0; i < mesh.num_vertices * mesh.v_components; i++) {
            tangentBufferRaw[i] = 0.f;
        }
        mesh.tangentBuffer = new float[3 * mesh.num_faces * mesh.v_components];
        mesh.texture_uv_buffer = new float[3 * mesh.num_faces * mesh.vt_components];
    }
    if (mesh.vn_components != -1) {
        normal_buffer_raw = new float[mesh.num_normal * mesh.vn_components];
        mesh.normal_buffer = new float[3 * mesh.num_faces * mesh.vn_components];
    } 

    mesh.vertex_buffer = new float[3 * mesh.num_faces * mesh.v_components];
    mesh.indices = new unsigned int[3 * mesh.num_faces];

    int loaded_vertex = 0, loaded_vt = 0, loaded_vn = 0, loaded_index = 0, loadedFace = 0;
    Vec3<float> p0, p1, p2;
    Vec2<float> uv0, uv1, uv2;
    
    // second pass
    while (std::getline(file, line)) {
        // Parse the line
        stringstream ss(line);
        
        if (line.find("v ") != string::npos) {
            ss >> dump; // dump the line prefix like v, vt, vn, f
            for (int i = 0; i < mesh.v_components; i++) {
                ss >> vertex_buffer_raw[loaded_vertex * mesh.v_components + i];  
            }
            loaded_vertex++;
        } else if (line[0] == 'v' && line[1] == 't') {
            ss >> dump;
            for (int i = 0; i < mesh.vt_components; i++) {
                ss >> texture_uv_buffer_raw[loaded_vt * mesh.vt_components + i];
            }
            loaded_vt++;
        } else if (line[0] == 'v' && line[1] == 'n') {
            ss >> dump;
            for (int i = 0; i < mesh.vn_components; i++) {
                ss >> normal_buffer_raw[loaded_vn * mesh.vn_components + i];
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
                uv0 = fetchTextureUVFromBuffer(texture_uv_buffer_raw, faceIndexArray[1] - 1, mesh.vt_components, 0);
                uv1 = fetchTextureUVFromBuffer(texture_uv_buffer_raw, faceIndexArray[4] - 1, mesh.vt_components, 0);
                uv2 = fetchTextureUVFromBuffer(texture_uv_buffer_raw, faceIndexArray[7] - 1, mesh.vt_components, 0);
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
            for (int i = 0; i < mesh.idx_components; i++) {
                unsigned int index;
                string token;
                unsigned int loaded_sub_component = 1;
                ss >> token;
                index = get_index(token);
                int nextVertex = loaded_index * mesh.v_components; 
                mesh.vertex_buffer[nextVertex] = vertex_buffer_raw[(index - 1) * mesh.v_components];  
                mesh.vertex_buffer[nextVertex + 1] = vertex_buffer_raw[(index - 1) * mesh.v_components + 1];  
                mesh.vertex_buffer[nextVertex + 2] = vertex_buffer_raw[(index - 1) * mesh.v_components + 2];  
                mesh.indices[loaded_index] = index;
            
                for ( ; loaded_sub_component < index_sub_component; loaded_sub_component++) {
                    switch (loaded_sub_component % index_sub_component) {
                        case 1: {
                            index = get_index(token);
                            mesh.texture_uv_buffer[loaded_index * mesh.vt_components] = texture_uv_buffer_raw[(index - 1) * mesh.vt_components];
                            mesh.texture_uv_buffer[loaded_index * mesh.vt_components + 1] = texture_uv_buffer_raw[(index - 1) * mesh.vt_components + 1];
                            mesh.texture_uv_buffer[loaded_index * mesh.vt_components + 2] = texture_uv_buffer_raw[(index - 1) * mesh.vt_components + 2];
                        } break;

                        case 2: {
                            index = get_index(token);
                            mesh.normal_buffer[loaded_index * mesh.vn_components] = normal_buffer_raw[(index - 1) * mesh.vn_components];
                            mesh.normal_buffer[loaded_index * mesh.vn_components + 1] = normal_buffer_raw[(index - 1) * mesh.vn_components + 1];
                            mesh.normal_buffer[loaded_index * mesh.vn_components + 2] = normal_buffer_raw[(index - 1) * mesh.vn_components + 2];
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
        for (int i = 0; i < mesh.num_vertices; i++) {
            Vec3<float> summedTangents(tangentBufferRaw[i * mesh.v_components],
                                       tangentBufferRaw[i * mesh.v_components + 1],
                                       tangentBufferRaw[i * mesh.v_components + 2]);
            Vec3<float> normalizedTangent = Math::normalize(summedTangents);
            tangentBufferRaw[i * mesh.v_components] = normalizedTangent.x;
            tangentBufferRaw[i * mesh.v_components + 1] = normalizedTangent.y;
            tangentBufferRaw[i * mesh.v_components + 2] = normalizedTangent.z;
        }
        // reorder the tangents to match vertex buffer
        for (int i = 0; i < mesh.num_faces * 3; i++) {
            int vertexIdx = (mesh.indices[i] - 1) * 3;
            mesh.tangentBuffer[i * 3] = tangentBufferRaw[vertexIdx];
            mesh.tangentBuffer[i * 3 + 1] = tangentBufferRaw[vertexIdx + 1];
            mesh.tangentBuffer[i * 3 + 2] = tangentBufferRaw[vertexIdx + 2];
        }
    }

    // Clean up
    delete []vertex_buffer_raw;
    delete []texture_uv_buffer_raw;
    delete []normal_buffer_raw;
    delete []tangentBufferRaw;
    delete []mesh.indices;
}

void SceneManager::loadSceneFromFile(Scene& scene, const char* filename) {
    nlohmann::json sceneJson;
    std::ifstream sceneFile(filename);
    sceneFile >> sceneJson;
    auto cameras = sceneJson["cameras"];
    auto lights = sceneJson["lights"];
    auto meshInfoList = sceneJson["mesh_info_list"];
    auto textureInfoList = sceneJson["texture_info_list"];
    auto instanceInfoList = sceneJson["instance_list"];
    // TODO: each scene should only have one camera
    for (auto camera : cameras) {
        scene.main_camera = camera.get<Camera>();
    }
    for (auto lightInfo : lights) {
        std::string lightType;
        DirectionalLight directionalLight;
        PointLight pointLight;
        lightInfo.at("type").get_to(lightType);
        if (lightType == "directional") {
            directionalLight.direction = Math::normalize(lightInfo.at("direction").get<Vec3<float>>());
            directionalLight.color = lightInfo.at("color").get<Vec3<float>>();
            scene.directionalLightList.emplace_back(directionalLight);
        } else {

        } // pointLight case
    }
    for (auto textureInfo : textureInfoList) {
        Texture texture;
        textureInfo.at("textureName").get_to(texture.textureName);
        textureInfo.at("texturePath").get_to(texture.texturePath);
        loadTextureFromFile(scene, texture.textureName, texture.texturePath.c_str());
    }
    for (auto meshInfo : meshInfoList) {
        std::string path;
        Mesh mesh;
        meshInfo.at("name").get_to(mesh.name);
        meshInfo.at("path").get_to(path);
        auto diffuseMaps = meshInfo.at("diffuseTexture");
        auto specularMaps = meshInfo.at("specularTexture");
        for (auto& diffuseMap : diffuseMaps) {
            std::string textureName;
            diffuseMap.get_to(textureName);
            if (textureName == "") {
                continue;
            }
            mesh.diffuseMapTable.insert(std::pair<std::string, int>(textureName, -1));
        }
        for (auto& specularMap : specularMaps) {
            std::string textureName;
            specularMap.get_to(textureName);
            if (textureName == "") {
                continue;
            }
            mesh.specularMapTable.insert(std::pair<std::string, int>(textureName, -1));
        }
        meshInfo.at("normalMapName").get_to(mesh.normalMapName);
        if (meshInfo.at("aoMapName") != "") {
            meshInfo.at("aoMapName").get_to(mesh.aoMapName);
        } 
        if (meshInfo.at("roughnessMapName") != "") {
            meshInfo.at("roughnessMapName").get_to(mesh.roughnessMapName);
        } 
        // binding diffuse map ids
        for(auto itr = mesh.diffuseMapTable.begin(); itr != mesh.diffuseMapTable.end(); itr++) {
            int idx = findTextureIndex(scene, itr->first);
            if (idx != -1) {
                itr->second = idx;
            }
        }
        // binding specular map ids
        for(auto itr = mesh.specularMapTable.begin(); itr != mesh.specularMapTable.end(); itr++) {
            int idx = findTextureIndex(scene, itr->first);
            if (idx != -1) {
                itr->second = idx;
            }
        }

        loadObj(mesh, path.c_str());
        findTexturesForMesh(scene, mesh);
        scene.mesh_list.emplace_back(mesh);
    }
    int instanceID = 0;
    for (auto instanceInfo : instanceInfoList) {
        std::string meshName;
        Mesh_Instance meshInstance = {};
        instanceInfo.at("meshName").get_to(meshName);
        auto xform_info = instanceInfo.at("xform");
        Transform xform = instanceInfo.at("xform").get<Transform>();
        for (int i = 0; i < scene.mesh_list.size(); i++) {
            if (scene.mesh_list[i].name == meshName) {
                meshInstance.instance_id = instanceID;
                meshInstance.mesh_id = i;
                scene.instance_list.emplace_back(meshInstance);
                scene.xform_list.emplace_back(xform);
                break;
            }
        }
        instanceID++;
    }
    if (sceneJson["hasSkybox"]) {
        std::string skyboxMeshName;
        auto skyboxMeshInfo = sceneJson["skyboxMesh"];
        skyboxMeshInfo.at("name").get_to(skyboxMeshName);
        std::cout << "scene has skybox" << std::endl;
        for (int i = 0; i < scene.mesh_list.size(); i++) {
            if (scene.mesh_list[i].name == skyboxMeshName) {
                scene.skyboxMeshID = i;
                break;
            }
        }
    } else {
        scene.skyboxMeshID = -1;
    }
}

void SceneManager::loadTextureFromFile(Scene& scene, std::string& name, const char* filename) {
    Texture newTexture = { };
    newTexture.textureName = name;
    newTexture.texturePath = filename;
    newTexture.pixels = stbi_load(filename, &newTexture.textureWidth, &newTexture.textureHeight, &newTexture.numChannels, 0);
    scene.textureList.emplace_back(newTexture);
}

int SceneManager::findTextureIndex(const Scene& scene, const std::string& textureName) {
    for (int i = 0; i < scene.textureList.size(); i++) {
            if (scene.textureList[i].textureName == textureName) {
                return i;
            }
    }
    return -1;
}

void SceneManager::findNormalMapForMesh(Scene& scene, Mesh& mesh) {
    for (int i = 0; i < scene.textureList.size(); i++) {
        if (scene.textureList[i].textureName == mesh.normalMapName) {
            mesh.normalMapID = i;
        }
    }
}

void SceneManager::findTexturesForMesh(Scene& scene, Mesh& mesh) {
    for (int i = 0; i < scene.textureList.size(); i++) {
        if (scene.textureList[i].textureName == mesh.normalMapName) {
            mesh.normalMapID = i;
        }
        if (scene.textureList[i].textureName == mesh.aoMapName) {
            mesh.aoMapID = i;
        }
        if (scene.textureList[i].textureName == mesh.roughnessMapName) {
            mesh.roughnessMapID = i;
        }
    }
}

// TODO: stop moving every object in the scene
void SceneManager::updateScene(Scene& scene, float deltaTime) {
    for (auto& instance : scene.instance_list) {
        scene.xform_list[instance.instance_id].rotation.x += 0.005f;
    }
}