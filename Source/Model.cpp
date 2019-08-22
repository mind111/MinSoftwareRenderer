#pragma once

#include "../Include/Model.h"

namespace Graphx {

    Model::Model() {
        this->VertexBuffer = nullptr;
        Indices = nullptr;
        NumOfVertices = 0;
        NumOfIndices = 0;
    }

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
            if (Line[0] == 'f' && Line[1] == ' ')
                NumOfIndices += 3;
        }
        /// \Note: Increment 1 here because wavefront .obj file vertex starting
        /// at index 1 instead of 0, so I will fill in the buffer at index 1 and
        /// leave index 0 as empty
        this->VertexBuffer = new Vec3<float>[NumOfVertices + 1];
        Vec3<float>* VBufferPtr = VertexBuffer + 1;
        this->Indices = new int[NumOfIndices + 1];
        int* IndexPtr = Indices + 1;
        // Reset the file pointer
        File.clear();
        File.seekg(0);
        while (File)
        {
            char Line[64];
            File.getline(Line, 64);
            char* CharPtr = Line;
            // Found a vertex
            if (Line[0] == 'v' && Line[1] == ' ')
            {
                bool NextLine = false;
                CharPtr += 2;
                int NumOfComponents = 0;
                while (CharPtr && !NextLine)
                {
                    char NumString[32];
                    char* NumCharPtr = NumString;
                    while (*CharPtr != ' ' && *CharPtr != '\0')
                    {
                        *NumCharPtr = *CharPtr;
                        CharPtr++;
                        NumCharPtr++;
                    }
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
                    CharPtr++;
                }
                VBufferPtr++;
            }

            // Reading Indices
            else if (Line[0] == 'f')
            {
                int VertexCount = 0;
                CharPtr += 2;
                while (CharPtr)
                {
                    char NumString[16];
                    char* NumCharPtr = NumString;
                    while (*CharPtr != '/')
                    {
                        *NumCharPtr = *CharPtr;
                        NumCharPtr++;
                        CharPtr++;
                    }
                    *NumCharPtr = '\0';
                    int Num = std::stoi(NumString);
                    *IndexPtr = Num;
                    IndexPtr++;
                    VertexCount++;
                    if (VertexCount == 3) break;
                    while (*CharPtr != ' ') CharPtr++;
                    CharPtr++;
                }
            }
        }
    }
}