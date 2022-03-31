
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_NV_viewport_array2: enable


layout(binding = 0) uniform UniformBufferObject {
    mat4 viewProjection[4];
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
layout (viewport_relative) out highp int gl_Layer;

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

    /* For MVR
    gl_layer is not to be read but to be writen to in this shader so somehow, out positions for all views must be written to in this shader

    so wrong again - not using layer but viewportMask to render to multiple views

    no will be rendeirng to multiple layers using this protocal:
    no dont do this this was for geom shader: set gl_posation, layer and others and set out varibles than call EmitStreamVertex() which emits so then you can render to later viewport layers

    */

    //need to only do this however many times for each window in group (i.e. 1 to 4)
    //for(int i = 0; i < 4; i++) {
        
        //not sure if these muse be reset each time or not
        


        // gl_Position = ubo.viewProjection[i] * modelUniform.data[drawData.modelIndex].model * vec4(inPosition, 1.0);
        // gl_Layer = i;
    //}

    
    gl_ViewportIndex = 1;
    // gl_ViewportMask[0] = 2;
    // gl_Layer = 0;
    gl_Position = ubo.viewProjection[1] * modelUniform.data[drawData.modelIndex].model * vec4(inPosition, 1.0);
}

//was in config - was getting weird freeing resources error wiht second window (not using mvr)
/*
,
    {
      "mode": "Windowed",
      "monitor": "\\\\.\\DISPLAY19\\Monitor0",
      "monitorLocalPostion": {
        "x": 0.7,
        "y": 0.4
      },
      "group": 1,
      "size": {
        "x": 1920,
        "y": 1080
      }
    }
    */