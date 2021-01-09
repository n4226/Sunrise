
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable


layout( push_constant ) uniform DrawPushData {
  uint modelIndex;
  uint matIndex;
} drawData;


struct MaterialUniforms {
    uint baseTextureIndex;
};

layout(binding = 2) buffer a_MaterialUniforms {
    MaterialUniforms data[];
} materialUniform;


layout(binding = 3) uniform sampler2D textures[];


layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 uvs;

layout(location = 0) out vec4 outAlbedo_Metallic;
layout(location = 1) out vec4 outNormal_Roughness;
layout(location = 2) out float outAO;

void main() {
    vec2 finalUvs = uvs * 0.1;
    
    MaterialUniforms mat = materialUniform.data[drawData.matIndex];

    vec3 color     = texture(textures[mat.baseTextureIndex  ],finalUvs).xyz;
    vec3 normal    = vec3(0);//texture(textures[mat.baseTextureIndex + 1],finalUvs).xyz;
    float metallic = 0;//texture(textures[mat.baseTextureIndex + 2],finalUvs).x;
    float ao       = 0;//texture(textures[mat.baseTextureIndex + 3],finalUvs).x;

    outAlbedo_Metallic = vec4(color, metallic);
    outNormal_Roughness = vec4(fragNormal, 1);
    outAO = ao;
}