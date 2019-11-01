#pragma once

#include "../include/Model.h"
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