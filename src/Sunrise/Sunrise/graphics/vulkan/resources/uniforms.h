#pragma once

#include "srpch.h"

namespace sunrise::gfx {


    struct TriangleUniformBufferObject {
        glm::mat4 model;
        glm::mat4 viewProjection;
    };

    struct SceneUniforms {
        //if no MVR than just first index is used
        glm::mat4 viewProjection[4];
    };

    struct PostProcessEarthDatAndUniforms {
        glm::mat4 invertedViewMat[4];
        glm::mat4 invertedProjMat[4];
        glm::mat4 viewMat[4];
        glm::mat4 projMat[4];
        glm::vec4 camFloatedGloabelPos[4];

        glm::vec4 earthCenter;
        glm::vec4 sunDir;
        glm::ivec2 renderTargetSize;
    };

    struct ModelUniforms {
        glm::mat4 model;
    };

    struct MaterialUniforms {
        glm::uint32_t baseTextureIndex;
    };

    struct DrawPushData
    {
        glm::uint32 modelIndex;
        glm::uint32 matIndex;
    };

}