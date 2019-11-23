#include <sstream>
#include "Model.h"

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