#pragma once

#include "srpch.h"

namespace sunrise::gfx {


    struct TriangleUniformBufferObject {
        glm::mat4 model;
        glm::mat4 viewProjection;
    };

    struct SceneUniforms {
        glm::mat4 viewProjection;
    };

    struct PostProcessEarthDatAndUniforms {
        glm::mat4 invertedViewMat;
        glm::mat4 invertedProjMat;
        glm::mat4 viewMat;
        glm::mat4 projMat;

        glm::vec4 earthCenter;
        glm::vec4 sunDir;
        glm::vec4 camFloatedGloabelPos;
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