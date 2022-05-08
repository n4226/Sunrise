#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable


layout( push_constant ) uniform DrawPushData {
  uint modelIndex;
  // i.e what material this will be this is the equivilent to setting material == grass
  uint matIndex;
} drawData;


struct ModelUniforms {
    mat4 model;
};

layout(binding = 1) buffer a_ModelUniforms {
    ModelUniforms data[];
} modelUniform;

struct MaterialUniforms {
// the texture i.e mem address of texture(s)
    uint baseTextureIndex;
};

layout(binding = 2) buffer a_MaterialUniforms {
    MaterialUniforms data[];
} materialUniform;


layout(binding = 3) uniform sampler2D textures[];


//layout(location = 0) in vec3 TBNMatrix;
// layout(location = 0) in vec3 fragModelNormal;
// layout(location = 1) in vec3 fragModelTangent;
// layout(location = 2) in vec3 fragModelBitangent;
layout(location = 0) in vec2 uvs;
layout(location = 1) in mat3 inTBN;

layout(location = 0) out vec4 outAlbedo_Metallic;
layout(location = 1) out vec4 outNormal_Roughness;
layout(location = 2) out float outAO;

vec3 srgb_to_linear(vec3 c) {
    return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), step(0.04045, c));
}

void main() {
    vec2 finalUvs = uvs * 0.1;
    
    MaterialUniforms mat = materialUniform.data[drawData.matIndex];

    vec3 color      = texture(textures[mat.baseTextureIndex  ],finalUvs).xyz;
    vec3 normal     = texture(textures[mat.baseTextureIndex + 1],finalUvs).xyz;
    float metallic  = texture(textures[mat.baseTextureIndex + 2],finalUvs).x;
    float ao        = texture(textures[mat.baseTextureIndex + 3],finalUvs).x;
    float roughness = texture(textures[mat.baseTextureIndex + 4],finalUvs).x;

    mat4 matGeo = modelUniform.data[drawData.modelIndex].model;

    // normal = srgb_to_linear(normal);
    // remape normals to correct domain to [-1,1]
    normal = normalize(normal * 2.0 - 1.0); 
    // normal.y = -normal.y;
    // normal.z = -normal.z;
    // normal.x = -normal.x;

    //temp - NORMAL MAPS STILL NOT WORKING OTHER PBR STUFF SHOULD BE
    normal = vec3(0,0,1);

    
    vec3 modelNormal = normalize(inTBN * normal);
    // in dommain [-1,1]
    //see: https://vulkanppp.wordpress.com/2017/07/06/week-6-normal-mapping-specular-mapping-pipeline-refactoring/
    vec3 worldNormal = modelNormal;//((matGeo * vec4(modelNormal,0)).xyz);
    //worldNormal = normal;
    //worldNormal = inTBN[0];

    worldNormal = normalize(worldNormal);
    // remap to domain [0,1]
    worldNormal = (worldNormal + 1.0) * 0.5;
    // worldNormal.xy = uvs;
    // worldNormal.z = 0;
    outAlbedo_Metallic = vec4(color,metallic);//vec4(color, metallic);
    outNormal_Roughness = vec4(worldNormal, roughness);
    outAO = ao;
}

