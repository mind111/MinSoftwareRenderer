#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "../Include/tgaimage.h"
#include "../Include/Math.h"

namespace Graphx
{
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
        // Index buffer
        Vec3<int>* Indices;
        
        TGAImage** TextureAssets;

        int NumOfLoadedTextures;
        int NumOfVertices;
        int NumOfIndices;
        int NumOfTextureUV;
        
        Model();
        ~Model();
        void Parse(char* FileName);
        bool LoadTexture(TGAImage* TextureImage, const char* FileName);
    };
}
