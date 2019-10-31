#include "../include/mesh.h"
#include <iostream>
#include <fstream>
#include <sstream>

Mesh::Mesh() {
    vertex_buffer = nullptr;
    texture_uv_buffer = nullptr;
    normal_buffer = nullptr;
    indices = nullptr;
    num_vertices = 0;
    num_texture_coord = 0;
    num_normal = 0;
    num_faces = 0;
    material_id = -1;
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
    int result = -1;
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

void Mesh::load_obj(const char* filename) {
    using std::string;
    using std::stringstream;

    std::ifstream file(filename);
    if (!file.is_open()) std::cerr << "Cannot open file: " << filename << std::endl;

    string line, dump, token;
    string delim = "/";
    int vertex_component = -1, vertex_normal_component = -1, texture_uv_component = 3, 
        index_component = -1, index_sub_component = -1;
    
    // first pass
    while (std::getline(file, line)) {
        stringstream ss(line);
        if (line[0] == 'v' && line[1] == ' ') {
            if (vertex_component == -1) {
                vertex_component = split_by_spaces(line) - 1;
            }
            this->num_vertices++;
        } else if (line[0] == 'v' && line[1] == 't') {
            if (texture_uv_component == -1) {
                texture_uv_component = split_by_spaces(line) - 1;
                texture_uv_buffer = new float [texture_uv_component * 3 * num_faces];
            }
            num_texture_coord++;
        } else if (line[0] == 'v' && line[1] == 'n') {
            if (vertex_normal_component == -1) {
                vertex_normal_component = split_by_spaces(line) - 1;
                normal_buffer = new float [vertex_normal_component * 3 * num_faces];
            }
            num_normal++;
        } else if (line[0] == 'f') {
            if (index_component == -1) {
                index_component = split_by_spaces(line) - 1;
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
    float* vertex_buffer_raw = new float[num_vertices * vertex_component];
    float* texture_uv_buffer_raw = 0; 
    float* normal_buffer_raw = 0;

    if (texture_uv_component != -1) {
        texture_uv_buffer_raw = new float[num_texture_coord * texture_uv_component];
        texture_uv_buffer = new float[3 * num_faces * texture_uv_component];
    }
    if (vertex_normal_component != -1) {
        normal_buffer_raw = new float[num_normal * vertex_normal_component];
        normal_buffer = new float[3 * num_faces * vertex_normal_component];
    } 

    vertex_buffer = new float[3 * num_faces * vertex_component];
    indices = new unsigned int[3 * num_faces];

    int loaded_vertex = 0, loaded_vt = 0, loaded_vn = 0, loaded_index = 0;
    
    // second pass
    while (std::getline(file, line)) {
        // Parse the line
        stringstream ss(line);
        
        if (line.find("v ") != string::npos) {
            ss >> dump; // dump the line prefix like v, vt, vn, f
            for (int i = 0; i < vertex_component; i++) {
                ss >> vertex_buffer_raw[loaded_vertex * vertex_component + i];  
            }
            loaded_vertex++;
        } else if (line[0] == 'v' && line[1] == 't') {
            ss >> dump;
            for (int i = 0; i < texture_uv_component; i++) {
                ss >> texture_uv_buffer_raw[loaded_vt * texture_uv_component + i];
            }
            loaded_vt++;
        } else if (line[0] == 'v' && line[1] == 'n') {
            ss >> dump;
            for (int i = 0; i < texture_uv_component; i++) {
                ss >> normal_buffer_raw[loaded_vn * vertex_normal_component + i];
            }
            loaded_vn++;
        } else if (line[0] == 'f') {
            ss >> dump;
            for (int i = 0; i < index_component; i++) {
                unsigned int index;
                string token, index_str;
                unsigned int loaded_sub_component = 1;
                ss >> token;
                index = get_index(token);
                vertex_buffer[loaded_index * vertex_component] = vertex_buffer_raw[(index - 1) * vertex_component];  
                vertex_buffer[loaded_index * vertex_component + 1] = vertex_buffer_raw[(index - 1) * vertex_component + 1];  
                vertex_buffer[loaded_index * vertex_component + 2] = vertex_buffer_raw[(index - 1) * vertex_component + 2];  

                for ( ; loaded_sub_component < index_sub_component; loaded_sub_component++) {
                    switch (loaded_sub_component % index_sub_component) {
                        case 1: {
                            index = get_index(token);
                            texture_uv_buffer[loaded_index * texture_uv_component] = texture_uv_buffer_raw[(index - 1) * texture_uv_component];
                            texture_uv_buffer[loaded_index * texture_uv_component + 1] = texture_uv_buffer_raw[(index - 1) * texture_uv_component + 1];
                            texture_uv_buffer[loaded_index * texture_uv_component + 2] = texture_uv_buffer_raw[(index - 1) * texture_uv_component + 2];
                        } break;

                        case 2: {
                            index = get_index(token);
                            normal_buffer[loaded_index * vertex_normal_component] = normal_buffer_raw[(index - 1) * vertex_normal_component];
                            normal_buffer[loaded_index * vertex_normal_component + 1] = normal_buffer_raw[(index - 1) * vertex_normal_component + 1];
                            normal_buffer[loaded_index * vertex_normal_component + 2] = normal_buffer_raw[(index - 1) * vertex_normal_component + 2];
                        } break;

                        default: {
                        }
                    }
                }
                loaded_index++;
            }
        }
    }

    // Clean up
    delete []vertex_buffer_raw;
    delete []texture_uv_buffer_raw;
    delete []normal_buffer_raw;
}
