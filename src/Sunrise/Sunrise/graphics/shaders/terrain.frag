
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


layout(location = 0) in vec3 fragModelNormal;
layout(location = 1) in vec3 fragModelTangent;
layout(location = 2) in vec3 fragModelBitangent;
layout(location = 3) in vec2 uvs;

layout(location = 0) out vec4 outAlbedo_Metallic;
layout(location = 1) out vec4 outNormal_Roughness;
layout(location = 2) out float outAO;

void main() {
    vec2 finalUvs = uvs * 0.1;
    
    MaterialUniforms mat = materialUniform.data[drawData.matIndex];

    vec3 color      = texture(textures[mat.baseTextureIndex  ],finalUvs).xyz;
    vec3 normal     = texture(textures[mat.baseTextureIndex + 1],finalUvs).xyz;
    float metallic  = texture(textures[mat.baseTextureIndex + 2],finalUvs).x;
    float ao        = texture(textures[mat.baseTextureIndex + 3],finalUvs).x;
    float roughness = texture(textures[mat.baseTextureIndex + 4],finalUvs).x;

    mat4 matGeo = modelUniform.data[drawData.modelIndex].model;

    // remape normals to correct domain
    normal = normal * 2.0 - 1.0;   

    //normal = vec3(0,0,1);


    //TODO: remove minus on bitang ? - the bitangents ahe been inverted in the way they are now generated
    mat3 TBN = mat3(fragModelTangent, fragModelBitangent, fragModelNormal);
    
    //vec3 worldNormal = fragModelNormal;

    //vec3 worldNormal = (matGeo * vec4(fragModelNormal,0)).xyz; 
    //color = vec3(uvs.x,uvs.y,1);

    //vec3 worldNormal = (matGeo * vec4(fragModelBitangent,0)).xyz;//TBN * normal,0)).xyz;
    
    vec3 worldNormal = (matGeo * vec4(TBN * normal,0)).xyz;
    //this was uncommented
    //vec3 worldNormal = (matGeo * vec4(TBN * (normal * vec3(0.6,0.6,1)),0)).xyz;

    outAlbedo_Metallic = vec4(color, metallic);
    outNormal_Roughness = vec4(worldNormal, roughness);
    outAO = ao;
}