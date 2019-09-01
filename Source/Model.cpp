#pragma once

#include "../Include/Model.h"

#define MAX_NUM_TEXTURE 5

namespace Graphx {

    Model::Model() 
    {
        this->VertexBuffer = nullptr;
        this->TextureBuffer = nullptr;
        this->Indices = nullptr;
        
        NumOfVertices = 0;
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

    ///\TODO: Consolidate this function, use bitmask to indicate
    //        which parsing mode e.g vertex, index, texuture we are in
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
                NumOfIndices += 3;
        }

        /// \Note: Increment 1 here because wavefront .obj file vertex starting
        /// at index 1 instead of 0, so I will fill in the buffer at index 1 and
        /// leave index 0 as empty
        this->VertexBuffer = new Vec3<float>[NumOfVertices + 1];
        this->TextureBuffer = new Vec2<float>[NumOfTextureUV + 1];
        this->Indices = new Vec3<int>[NumOfIndices];
        Vec3<float>* VBufferPtr = VertexBuffer + 1;
        Vec2<float>* VTBufferPtr = TextureBuffer + 1;
        Vec3<int>* IndexPtr = Indices;

        // Reset the file pointer
        File.clear();
        File.seekg(0);

        void* BufferPtr = nullptr;
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
                                VBufferPtr->z = Num;
                                NextLine = true;
                                break;
                            case 1:
                                VBufferPtr->x = Num;
                                break;
                            case 2:
                                VBufferPtr->y = Num;
                                break;
                            default:
                                break;
                        }
                        NumCharPtr = NumString;
                    }

                    CharPtr++;
                }

                VBufferPtr++;
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
                                VTBufferPtr->y = Num;
                                NextLine = true;
                                break;
                            case 1:
                                VTBufferPtr->x = Num;
                                break;
                            default:
                                break;
                        }
                        NumCharPtr = NumString;
                    }

                    CharPtr++;
                }

                VTBufferPtr++;
            }
        }
    }
}
