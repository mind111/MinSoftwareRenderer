#pragma once

#include "../Include/Model.h"
#include <sstream>

#define MAX_NUM_TEXTURE 5


Model::Model() 
{
    this->VertexBuffer = nullptr;
    this->TextureBuffer = nullptr;
    this->Indices = nullptr;
    
    NumOfVertices = 0;
    NumOfFaces = 0;
    NumOfIndices = 0;
    NumOfTextureUV = 0;

    this->TextureAssets = new TGAImage*[MAX_NUM_TEXTURE];

    for (int i = 0; i < MAX_NUM_TEXTURE; i++)
        this->TextureAssets[i] = nullptr;    
}

Model::~Model() 
{
    delete []this->VertexBuffer;
    delete []this->TextureBuffer;
    delete []this->Indices;
}

bool Model::LoadTexture(TGAImage* TextureImage, const char* FileName)
{
    int i = 0;
    if (!TextureImage->read_tga_file(FileName)) return false;
    while (this->TextureAssets[i]) i++;
    if (i >= MAX_NUM_TEXTURE)
    {
        std::cerr << "Cannot load more textures." << std::endl;
        return false;
    }
    TextureAssets[i] = TextureImage;    
    return true;
}

bool Model::LoadNormalMap(TGAImage* NormalMap, const char* FileName)
{
    if (!NormalMap->read_tga_file(FileName)) return false;
    this->NormalTexture = NormalMap; 
    return true;
}


///\TODO: This function is highly coupled with a specific type of .obj file.
///       need a lot of improvements
void Model::Parse(char* FileName)
{
    std::ifstream File(FileName);
    if (!File.is_open()) std::cerr << "Cannot open the file!" << std::endl;

    while (File)
    {
        char Line[64];
        File.getline(Line, 64);
        if (Line[0] == 'v' && Line[1] == ' ')
            NumOfVertices++;
        if (Line[0] == 'v' && Line[1] == 't')
            NumOfTextureUV++;
        if (Line[0] == 'f' && Line[1] == ' ')
        {
            NumOfFaces += 1;
            NumOfIndices += 3;
        }
    }

    /// \Note: Increment 1 here because wavefront .obj file vertex starting
    /// at index 1 instead of 0, so I will fill in the buffer at index 1 and
    /// leave index 0 as empty
    this->VertexBuffer = new Vec3<float>[NumOfVertices + 1];
    this->VertexNormalBuffer = new Vec3<float>[NumOfVertices + 1];
    this->TextureBuffer = new Vec2<float>[NumOfTextureUV + 1];
    this->Indices = new Vec3<int>[NumOfIndices];
    Vec3<float>* BufferPtr_Vertex = VertexBuffer + 1;
    Vec2<float>* BufferPtr_TextureUV = TextureBuffer + 1;
    Vec3<float>* BufferPtr_VertexNormal = VertexNormalBuffer + 1;
    Vec3<int>* IndexPtr = Indices;

    // Reset the file pointer
    File.clear();
    File.seekg(0);

    ParseMode Mode;

    while (File)
    {
        char Line[64];
        File.getline(Line, 64);
        char* CharPtr = Line;
        char NumString[32];
        char* NumCharPtr = NumString;
        int NumOfComponents = 0;
        bool NextLine = false;

        // Found a vertex
        if (Line[0] == 'v' && Line[1] == ' ')
        {
            Mode = ParseMode::Vertex_Mode;
            CharPtr += 2; // This line may change to skip all the leading spaces
            while (CharPtr && *CharPtr == ' ') CharPtr++;
            while (!NextLine)
            {
                if (CharPtr && *CharPtr != ' ' && *CharPtr != '\0')
                {
                    *NumCharPtr = *CharPtr;
                    NumCharPtr++;
                }

                else
                {
                    *NumCharPtr = '\0';

                /// \TODO: Research about fast method of converting a string to integer and float
                    float Num = std::stof(NumString);
                    NumOfComponents++;
                    switch (NumOfComponents % 3)
                    {
                        case 0:
                            BufferPtr_Vertex->z = Num;
                            NextLine = true;
                            break;
                        case 1:
                            BufferPtr_Vertex->x = Num;
                            break;
                        case 2:
                            BufferPtr_Vertex->y = Num;
                            break;
                        default:
                            break;
                    }
                    NumCharPtr = NumString;
                }

                CharPtr++;
            }

            BufferPtr_Vertex++;
        }

        ///\TODO: This is dumb, need to rewrite
        // Reading Indices
        else if (Line[0] == 'f')
        {
            CharPtr += 2;
            while (CharPtr && *CharPtr == ' ') CharPtr++;
            while (true)
            {
                Mode = ParseMode::Index_Mode;

                *NumCharPtr = *CharPtr;
                NumCharPtr++;
                CharPtr++;

                if (*CharPtr == '/' || *CharPtr == ' ' || *CharPtr == '\0')
                {
                    *NumCharPtr = '\0';
                    int Num = std::stoi(NumString);
                    NumOfComponents++;

                    switch(NumOfComponents % 3)
                    {
                        case 0:                // Vertex normal
                            IndexPtr->z = Num;
                            IndexPtr++; 
                            break;
                        case 1:                // Vertex pos 
                            IndexPtr->x = Num;
                            break;
                        case 2:                // Vertex uv
                            IndexPtr->y = Num;
                            break;
                        default:
                            break;
                    }
                    
                    NumCharPtr = NumString;
                    if (*CharPtr == '\0') break;
                    CharPtr++;
                }
            }
        }

        else if (Line[0] == 'v' && Line[1] == 't')
        {
            Mode = ParseMode::TextureUV_Mode;
            CharPtr += 2;
            while (CharPtr && *CharPtr == ' ') CharPtr++;
            while (!NextLine)
            {
                if (CharPtr && *CharPtr != '\0' && *CharPtr != ' ')
                {
                    *NumCharPtr = *CharPtr;
                    NumCharPtr++;
                }

                else
                {
                    *NumCharPtr = '\0';
                    float Num = std::stof(NumString);
                    NumOfComponents++;
                    switch(NumOfComponents % 2)
                    {
                        case 0:
                            BufferPtr_TextureUV->y = Num;
                            NextLine = true;
                            break;
                        case 1:
                            BufferPtr_TextureUV->x = Num;
                            break;
                        default:
                            break;
                    }
                    NumCharPtr = NumString;
                }

                CharPtr++;
            }

            BufferPtr_TextureUV++;
        }

        else if (Line[0] == 'v' && Line[1] == 'n')
        {

            Mode = ParseMode::VertexNormal_Mode;
            CharPtr += 2;
            while (CharPtr && *CharPtr == ' ') CharPtr++;
            while (!NextLine)
            {
                if (CharPtr && *CharPtr != ' ' && *CharPtr != '\0')
                {
                    *NumCharPtr = *CharPtr;
                    NumCharPtr++;
                }

                else
                {
                    *NumCharPtr = '\0';

                /// \TODO: Research about fast method of converting a string to integer and float
                    float Num = std::stof(NumString);
                    NumOfComponents++;
                    switch (NumOfComponents % 3)
                    {
                        case 0:
                            BufferPtr_VertexNormal->z = Num;
                            NextLine = true;
                            break;
                        case 1:
                            BufferPtr_VertexNormal->x = Num;
                            break;
                        case 2:
                            BufferPtr_VertexNormal->y = Num;
                            break;
                        default:
                            break;
                    }
                    NumCharPtr = NumString;
                }

                CharPtr++;
            }

           BufferPtr_VertexNormal++;
        }
    }
}

Mesh::Mesh() {
    vertex_buffer = nullptr;
    texture_uv_buffer = nullptr;
    normal_buffer = nullptr;
    indices = nullptr;
    num_vertices = 0;
    num_faces = 0;
    texture_id = -1;
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
        } else if (line[0] == 'v' && line[1] == 'n') {
            if (vertex_normal_component == -1) {
                vertex_normal_component = split_by_spaces(line) - 1;
                normal_buffer = new float [vertex_normal_component * 3 * num_faces];
            }
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
        texture_uv_buffer_raw = new float[num_vertices * texture_uv_component];
        texture_uv_buffer = new float[3 * num_faces * texture_uv_component];
    }
    if (vertex_normal_component != -1) {
        normal_buffer_raw = new float[num_vertices * vertex_normal_component];
        normal_buffer = new float[3 * num_faces * vertex_normal_component];
    } 

    vertex_buffer = new float[3 * num_faces * vertex_component];
    indices = new unsigned int[3 * num_faces];

    int loaded_vertex = 0, loaded_index = 0;
    
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

    delete []vertex_buffer_raw;
    delete []texture_uv_buffer_raw;
    delete []normal_buffer_raw;
}

Cubemap::Cubemap() {
    vertex_buffer = nullptr;
    indicies = nullptr;
}

// Assuming that Cubemap has 6 textures by default
void Cubemap::load_cubemap_textures(const char** texture_paths) {
    for (int i = 0; i < 6; i++) {
        texture_assets[i].read_tga_file(texture_paths[i]);
    }
}