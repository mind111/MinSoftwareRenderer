#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "../Include/tgaimage.h"
#include "../Include/Math.h"

// TODO: Rewrite Mesh class, and .obj loader
// TODO: Substitude current scene data

enum class ParseMode : int8_t
{
    Vertex_Mode,
    Index_Mode,
    TextureUV_Mode,
    VertexNormal_Mode
};

class Model
{
public:
    // Vertex buffer
    Vec3<float>* VertexBuffer;
    // Texture Coordinate buffer
    Vec2<float>* TextureBuffer;
    // Vertex Normal buffer
    Vec3<float>* VertexNormalBuffer;
    // Index buffer
    Vec3<int>* Indices;
    
    TGAImage** TextureAssets;
    TGAImage* NormalTexture;

    int NumOfLoadedTextures;
    int NumOfVertices;
    int NumOfFaces;
    int NumOfIndices;
    int NumOfTextureUV;
    
    Model();
    ~Model();
    void Parse(char* FileName);
    bool LoadTexture(TGAImage* TextureImage, const char* FileName);
    bool LoadNormalMap(TGAImage* NormalMap, const char* FileName);
};

class Mesh {
public:
    std::string name;
    float* vertex_buffer;
    float* texture_uv_buffer;
    float* normal_buffer;
    unsigned int* indices; // do not need index buffer if rearrange the vertex

    int num_vertices;
    int num_faces;
    int texture_id;

    Mesh();
    void load_obj(const char* filename);
    void load_texture(const char* filename);
};

class Model_Reboot {
    std::vector<Mesh> meshes;
};

class Cubemap {
public:
    float* vertex_buffer;
    unsigned int* indicies;
    TGAImage texture_assets[6];

    Cubemap();
    void load_cubemap_textures(const char** texture_paths);
};

