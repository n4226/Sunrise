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
            {0,1},
            {1,0},
            {1,1},
        };

        square.normals = {
            {0,0,1},
            {0,0,1},
			{0,0,1},
			{0,0,1},
        };
        
        square.tangents = {
            {1,0,0},
            {1,0,0},
            {1,0,0},
            {1,0,0},
        };
        
        square.bitangents = {
            {0,1,0},
            {0,1,0},
            {0,1,0},
            {0,1,0},
        };

        square.indicies = { {0,1,2,2,3,0} };

        square.calculateTangentsAndBitangents();

        return square;
    }

	Mesh SUNRISE_API MeshPrimatives::cube()
	{
		Mesh cube{};

        cube.verts = {
            glm::vec3(0, 0, 0) + glm::vec3(- 0.5,-0.5,-0.5),
			glm::vec3(0, 0, 0) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(0, 0, 0) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(0, 1, 0) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(0, 1, 0) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(0, 1, 0) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(1, 1, 0) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(1, 1, 0) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(1, 1, 0) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(1, 0, 0) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(1, 0, 0) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(1, 0, 0) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(0, 0, 1) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(0, 0, 1) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(0, 0, 1) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(0, 1, 1) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(0, 1, 1) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(0, 1, 1) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(1, 1, 1) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(1, 1, 1) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(1, 1, 1) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(1, 0, 1) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(1, 0, 1) + glm::vec3(- 0.5,-0.5,-0.5),
            glm::vec3(1, 0, 1) + glm::vec3(- 0.5,-0.5,-0.5),
		};

        //f6 is one of the side faces
        cube.uvs = {
			glm::vec2(0, 0),//f1
			glm::vec2(0, 0),//f2
			glm::vec2(0, 0),//f3
			glm::vec2(0, 1),//f1
			glm::vec2(0, 0),//f5
			glm::vec2(0, 1),//f3
			glm::vec2(0, 0),//f4
			glm::vec2(0, 1),//f5
			glm::vec2(1, 0),//f3
			glm::vec2(0, 1),//f4
			glm::vec2(1, 0),//f2
			glm::vec2(1, 1),//f3
			glm::vec2(1, 0),//f1
			glm::vec2(0, 1),//f2
			glm::vec2(1, 1),//f6
			glm::vec2(1, 1),//f1
			glm::vec2(1, 0),//f5
			glm::vec2(1, 0),//f6
			glm::vec2(1, 0),//f4
			glm::vec2(1, 1),//f5
			glm::vec2(0, 0),//f6
			glm::vec2(1, 1),//f4
			glm::vec2(1, 1),//f2
			glm::vec2(0, 1),//f6
		};

        cube.normals = {
			{-1,  0,  0},// < !-- 0 a(faces: 2,4,6)-- >
	        { 0, -1,  0},//
	        { 0,  0, -1},//
	        {-1,  0,  0},// < !-- 3 b(faces: 2,3,6)-- >
	        { 0,  1,  0},//
	        { 0,  0, -1},//
	        { 1,  0,  0},// < !-- 6 c(faces: 1,3,6)-- >
	        { 0,  1,  0},//
	        { 0,  0, -1},//
	        { 1,  0,  0},// < !-- 9 d(faces: 1,4,6)-- >
	        { 0, -1,  0},//
	        { 0,  0, -1},//
	        {-1,  0,  0},// < !-- 12 e(faces: 2,4,5)-- >
	        { 0, -1,  0},//
	        { 0,  0,  1},//
	        {-1,  0,  0},// < !-- 15 f(faces: 2,3,5)-- >
	        { 0,  1,  0},//
	        { 0,  0,  1},//
	        { 1,  0,  0},// < !-- 18 g(faces: 1,3,5)-- >
	        { 0,  1,  0},//
	        { 0,  0,  1},//
	        { 1,  0,  0},// < !-- 21 h(faces: 1,4,5)-- >
	        { 0, -1,  0},//
            { 0,  0,  1},//
		};

        cube.indicies = { {
			 //< !--Face 1: cghd = cgh + chd-->
	         6, 18, 21,
	         6, 21, 9,
             //< ,!--,Face 2: aefb = aef + afb-- >
	         0, 12, 15,
	         0, 15, 3,
	         //< ,!--,Face 3: gcbf = gcb + gbf-- >
	         19, 7, 4,
	         19, 4, 16,
	         //< ,!--,Face 4: dhea = dhe + dea-- >
	         10, 22, 13,
	         10, 13, 1,
	         //< ,!--,Face 5: hgfe = hgf + hfe-- >
	         23, 20, 17,
	         23, 17, 14,
	         //< ,!--,Face 6: cdab = cda + cab-- >
	         8, 11, 2,
             8, 2 ,5,
            } };

        cube.calculateTangentsAndBitangents();

		return cube;
	}



    Basic2DMesh SUNRISE_API MeshPrimatives::Basic2D::screenQuad()
    {
        Basic2DMesh mesh{};

        mesh.verts = {
            glm::vec2(-1,-1),
            glm::vec2(1,-1),
            glm::vec2(-1,1),
            glm::vec2(1,1),
        };

        mesh.indicies = {
            0,1,2,
            2,1,3,
        };

        return mesh;
    }

}