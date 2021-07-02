
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 viewProjection;
} ubo;

struct ModelUniforms {
    mat4 model;
};

layout(binding = 1) buffer a_ModelUniforms {
    ModelUniforms data[];
} modelUniform;

layout( push_constant ) uniform DrawPushData {
  uint modelIndex;
  uint matIndex;
} drawData;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(location = 0) out vec3 outFragModelNormal;
layout(location = 1) out vec3 outFragModelTangent;
layout(location = 2) out vec3 outFragModelBitangent;
layout(location = 3) out vec2 outUvs;

//vec2 positions[3] = vec2[](
//    vec2(0.0, -0.5),
//    vec2(0.5, 0.5),
//    vec2(-0.5, 0.5)
//);
//
//vec3 colors[3] = vec3[](
//    vec3(1.0, 0.3, 0.0),
//    vec3(0.2, 1.0, 0.0),
//    vec3(0.0, 0.1, 1.0)
//);

void main() {
    outUvs = inUv;
    outFragModelNormal = inNormal;
    outFragModelTangent = inTangent;
    outFragModelBitangent = inBitangent;

    // convert normal to worldspace
    //outWorldNormal = (modelUniform.data[drawData.modelIndex].model * vec4(inNormal,0)).xyz;
    gl_Position = ubo.viewProjection * modelUniform.data[drawData.modelIndex].model * vec4(inPosition, 1.0);
}