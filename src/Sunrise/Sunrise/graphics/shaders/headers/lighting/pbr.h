#pragma once



#include "../base.h"


#ifdef SR_PLATFORM_WINDOWS
#include <glm/glm.hpp>
using namespace glm;
#endif // SR_PLATFORM_WINDOWS

// PBR MATH

// types

//enum lightType {
//    lightTypePoint = 0,
//    lightTypeDir = 1
//};

struct Light {

    // see LightType
    int type;

    vec3 intensity;
    vec3 position;
    vec3 direction;

};


struct SampledPBRMaterial {
    vec3 albedo;
    vec3 worldSpaceNormals;
    float metallic;
    float ao;
    float roughness;
};

//

float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

//todo suport multiple lights in the future 
// see good pbr video explination: https://www.youtube.com/watch?v=j-A0mwsJRmk
vec3 light(SampledPBRMaterial mat, Light lights, vec3 fragWorldNormal, vec3 fragWorldPos, vec3 camWorldPos) { 

    // frage normal vector
    vec3 N = normalize(fragWorldNormal);
    // view vector
    vec3 V = normalize(camWorldPos - fragWorldPos);

    // light somethign something
    vec3 Lo = vec3(0.0);
    //    for(int i = 0; i < 4; ++i) {
    Light light = lights;

    vec3 radiance;
    // halfway vector
    vec3 H;
    // light direction vector e.g direction of the light for dir lights or the direction from the fragment to the light for points (the one for points seems backwords)
    vec3 L;

    //if (light.type == lightTypePoint) {
    //    // calc constants
    //    xhalf3 lightPos = xhalf3(light.position);
    //    L = normalize(lightPos - worldPos);
    //    H = normalize(V + L);

    //    xhalf distance = length(lightPos - worldPos);
    //    xhalf attenuation = 1.0 / (distance * distance);
    //    radiance = xhalf3(light.intensity) * attenuation;
    //}
    //else {
    //todo for now all lights are directional
        radiance = light.intensity;
        L = normalize(light.direction);
        H = normalize(V + L);
    //}

    //temp
        //return ((radiance * max(dot(N, L), 0.0f)) + 0.03f) * mat.albedo;

    // calc fresnel, dist, and geometry
    
    //the base ___ see pbr video: https://www.youtube.com/watch?v=j-A0mwsJRmk
    vec3 F0 = vec3(0.04);

    // this is because metals do not have a base defuse color but non metals do
    F0 = mix(F0, vec3(mat.albedo), vec3(mat.metallic));
    
    // the angle theta is betwen the halfay direciton (normal of the surface) and the view direction
    float cosTheta = max(dot(H, V), float(0.0));
   
    // fernel_Schlick value
    vec3 F = fresnelSchlick(cosTheta, F0);

    // normal distribution
    float NDF = DistributionGGX(N, H, mat.roughness);

    float G = GeometrySmith(N, V, L, mat.roughness);

    // calc cook-torrance BRDF -- putting it all together

    //  previus values put together
    vec3 numerator = NDF * G * F;
    // some transformation corrections
    float denominator = 4.0 * max(dot(N, V), float(0.0)) * max(dot(N, L), float(0.0));
    vec3 specular = numerator / max(denominator, float(0.001));


    // final calcs
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - mat.metallic;

    float NdotL = max(dot(N, L), float(0.0));

    // add the contribution of this light to all the lights
    Lo += (kD * vec3(mat.albedo) / PI_f + specular) * radiance * NdotL;
    //}

    // calculaitons on total Lo

    vec3 ambient = vec3(0.03) * vec3(mat.albedo) * mat.ao;
    vec3 color = ambient + Lo;


    return color;

}


// aces tone mapping 

//=================================================================================================
//
//  Baking Lab
//  by MJP and David Neubelt
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//=================================================================================================

// The code in this file was originally written by Stephen Hill (@self_shadow), who deserves all
// credit for coming up with this fit and implementing it. Buy him a beer next time you see him. :)

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
mat3x3 ACESInputMat()
{
    return mat3x3(vec3(0.59719, 0.35458, 0.04823),
        vec3(0.07600, 0.90834, 0.01566),
        vec3(0.02840, 0.13383, 0.83777));
}

// ODT_SAT => XYZ => D60_2_D65 => sRGB
mat3x3 ACESOutputMat()
{
    return mat3x3(vec3(1.60475, -0.53108, -0.07367),
        vec3(-0.10208, 1.10813, -0.00605),
        vec3(-0.00327, -0.07276, 1.07602));
}

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 ACESFitted(vec3 color)
{
    color = ACESInputMat() * color;

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = ACESOutputMat() * color;

    // Clamp to [0, 1]
    color = clamp(color, 0, 1);

    return color;
}


vec3 calculateLighting(
                      vec2 texCoards,
                      float depth,
                       mat4x4 inverseProj,
                       mat4x4 inverseView,
                      SampledPBRMaterial mat,
                        vec3 sunDir,
                        vec3 worldCamPos
                        ) {
        
    // really good copmarison of ndc spaces - https://github.com/gpuweb/gpuweb/issues/416
    // re create frag wold pos
    float z = depth;
    vec4 clipSpaceFragPos = vec4(texCoards * 2.0f - 1.0f, z, 1.0);

    // dont think i need this but i dont know;
    //clipSpaceFragPos.y = -clipSpaceFragPos.y;


    vec4 viewSpaceFragPosition =
        inverseProj * clipSpaceFragPos;
    
    // perspective division
    viewSpaceFragPosition /= viewSpaceFragPosition.w;

    vec4 worldSpaceFragPosition =
        inverseView * viewSpaceFragPosition;

    Light sunLight;

    sunLight.type = 1;
    sunLight.direction = sunDir;
    sunLight.intensity = vec3(7);//vec3(4);//vec3(8);

    //return vec3(3);
    //return worldSpaceFragPosition.xyz;
    vec3 color = light(mat, sunLight, mat.worldSpaceNormals, worldSpaceFragPosition.xyz, worldCamPos);


    // todo add ACES tone mapping here or better in another post pass
    //color = ACESFitted(color);

    return color;
    //return vec3(mat.worldSpaceNormals.xyz);
}
