#include "srpch.h"
#include "MeshPrimatives.h"

namespace sunrise {


    Mesh sunrise::MeshPrimatives::square()
    {
        Mesh square{};

        square.verts = {
            {-0.5,-0.5,0},
            {0.5,-0.5,0},
            {0.5,0.5,0},
            {-0.5,0.5,0},
        };

        square.uvs = {
            {0,0},
            {1,0},
            {1,1},
            {0,1},
        };

        square.normals = {
            {0,0,0},
            {0,0,0},
            {0,0,0},
        };
        
        square.tangents = {
            {0,0,0},
            {0,0,0},
            {0,0,0},
        };
        
        square.bitangents = {
            {0,0,0},
            {0,0,0},
            {0,0,0},
        };

        square.indicies = { {0,1,2,2,3,0} };

        return square;
    }

}