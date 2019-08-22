#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include "../Include/Math.h"

namespace Graphx
{
    class Model
    {
    public:
        // Vertex buffer
        Vec3<float>* VertexBuffer;
        // Index buffer
        int* Indices;
        int NumOfVertices;
        int NumOfIndices;

        Model();
        void Parse(char* FileName);
    };
}
