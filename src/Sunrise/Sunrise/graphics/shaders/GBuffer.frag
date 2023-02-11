#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_NV_viewport_array2: enable

#include "headers/lighting/atmScat.h"
// #include "headers/lighting/pbr.h"
#include "headers/lighting/pbr.h"


//TODO: move this back to binding 4 when depth buffer added back
layout(binding = 4) uniform UniformBufferObject {
// global uniforms
    mat4 viewProjection[4];

    // post uniforms
    mat4 invertedViewMat[4];
    mat4 invertedProjMat[4];
    mat4 viewMat[4];
    mat4 projMat[4];
    vec4 camFloatedGloabelPos[4];

    vec4 earthCenter;
    vec4 sunDir;
    ivec2 renderTargetSize;
} ubo;







//
//layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput GBuffer_Albedo_Metallic;
//layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput GBuffer_Normal_Roughness;
//layout (input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput GBuffer_AO;
//layout (input_attachment_index = 3, set = 0, binding = 3) uniform subpassInput GBuffer_Depth;


layout (binding = 0) uniform sampler2D GBuffer_Albedo_Metallic;
layout (binding = 1) uniform sampler2D GBuffer_Normal_Roughness;
layout (binding = 2) uniform sampler2D GBuffer_AO;
layout (binding = 3) uniform sampler2D GBuffer_Depth;

float remap(float value, float low1,float high1,float low2,float high2) {
    return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
}

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 inPos;

void main() {
    

	//TODO can calculate this in vertex to be more efficient
    // second param is lod
    // inPos is in rnage [-0.5,0.5]
    // uvs are in range [0,1]
	vec2 uvs = (inPos + 1) / 2;// / vec2(ubo.renderTargetSize);
    vec4 albedo_metallic =   texture(GBuffer_Albedo_Metallic,uvs);
    vec4 normal_sroughness = texture(GBuffer_Normal_Roughness,uvs);
    float ao =               texture(GBuffer_AO,uvs).x;
    float depth =            texture(GBuffer_Depth,uvs).x; 
    //albedo_metallic = normal_sroughness;

    //remap normals to domain [-1,1]
    normal_sroughness.xyz = normalize((normal_sroughness.xyz * 2) - 1);

    vec3 color;

    if (depth != 1) {
        SampledPBRMaterial mat;
        mat.albedo = albedo_metallic.xyz;
        mat.worldSpaceNormals = normal_sroughness.xyz;
        mat.metallic = albedo_metallic.w;
        mat.ao = ao.x;
        mat.roughness = normal_sroughness.w;
        
        //i had acidentally switched projeciton and view matrices inputed into this function
        color = calculateLighting(uvs,depth,ubo.invertedProjMat[gl_Layer],ubo.invertedViewMat[gl_Layer],mat,ubo.sunDir.xyz,ubo.camFloatedGloabelPos[gl_Layer].xyz);
    }else {
        color = vec3(0);
        // color.xyz = calculatePostAtmosphereicScatering(
        //     inPos.xy,ubo.camFloatedGloabelPos[gl_Layer].xyz,
        //     ubo.invertedProjMat[gl_Layer],ubo.invertedViewMat[gl_Layer],
        //     ubo.sunDir.xyz,ubo.earthCenter.xyz,color.xyz,depth);
        // color.xyz = vec3(0.1,0.9,0.2);
    }

    if (depth > 0.9) {
        color.xyz = calculatePostAtmosphereicScatering(
            inPos.xy,ubo.camFloatedGloabelPos[gl_Layer].xyz,
            ubo.invertedProjMat[gl_Layer],ubo.invertedViewMat[gl_Layer],
            ubo.sunDir.xyz,ubo.earthCenter.xyz,color.xyz,depth);
    }
    //atm skybox 
        

 // todo add ACES tone mapping in a better place such as post pass
    // color = ACESFitted(color);

    outColor = vec4(color, 1);
}