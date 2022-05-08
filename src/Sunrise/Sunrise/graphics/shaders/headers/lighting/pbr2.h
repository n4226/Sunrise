#pragma once

#include "../base.h"


#ifdef SR_PLATFORM_WINDOWS
#include <glm/glm.hpp>
using namespace glm;
#endif // SR_PLATFORM_WINDOWS

struct Light {
    // see LightType
    int type;

    vec3 intensity; // intensity of the light used for all types
    vec3 position; //not used for directional light
    vec3 direction; //not used for point light

};


struct SampledPBRMaterial {
    vec3 albedo;
    vec3 worldSpaceNormals;
    float metallic;
    float ao;
    float roughness;
};

//constrants

const float Epsilon = 0.00001;
const vec3 Fdielectric = vec3(0.04);


//base functions

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float roughness)
{
	float alpha   = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor.
vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}


//see: https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/data/shaders/pbr_khr.frag
//see helpful github demoRepo: https://github.com/Nadrin/PBR
    //https://github.com/Nadrin/PBR/blob/master/data/shaders/glsl/pbr_fs.glsl
//uses a physically based cook torance brdf
//all calcls in world space



/* important questions that need to be answered
- are the normals being clamped from when they are exported in gbuffer pass and imported in lighting pass?
    could have to do with negative values beimg messed up

*/
vec3 light(SampledPBRMaterial mat, Light lights, vec3 fragWorldPos, vec3 camWorldPos) {


	// Outgoing light direction (vector from world-space fragment position to the "eye").
	vec3 Lo = normalize(camWorldPos - fragWorldPos);

    // Get curresnt fragment's normal and transform to world space.
	vec3 N = mat.worldSpaceNormals;

    // Angle between surface normal and outgoing light direction.
	float cosLo = max(0.0, dot(N, Lo));

    // Specular reflection vector.
	vec3 Lr = 2.0 * cosLo * N - Lo;

    // Fresnel reflectance at normal incidence (for metals use albedo color).
	vec3 F0 = mix(Fdielectric, mat.albedo, mat.metallic);

    // Direct lighting calculation for analytical lights.
	vec3 directLighting = vec3(0);

    // Loop over all lights. - for now there is just one - the sun
    {
        vec3 Li = lights.direction;
		vec3 Lradiance = lights.intensity;

        // Half-vector between Li and Lo.
		vec3 Lh = normalize(Li + Lo);

        
		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0, dot(N, Li));
		float cosLh = max(0.0, dot(N, Lh));

        // Calculate Fresnel term for direct lighting. 
		vec3 F  = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
		// Calculate normal distribution for specular BRDF.
		float D = ndfGGX(cosLh, mat.roughness);
		// Calculate geometric attenuation for specular BRDF.
		float G = gaSchlickGGX(cosLi, cosLo, mat.roughness);
        // return vec3(F);

        // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
		// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
		// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
		vec3 kd = mix(vec3(1.0) - F, vec3(0.0), mat.metallic);

        // Lambert diffuse BRDF.
		// We don't scale by 1/PI for lighting & material units to be more convenient.
		// See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
		vec3 diffuseBRDF = kd * mat.albedo;


        // Cook-Torrance specular microfacet BRDF.
		vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

		// Total contribution for this light.
		directLighting += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
    }

    // Ambient lighting (IBL).
	vec3 ambientLighting;

    //TODO: add ambient here
    ambientLighting = vec3(0.03);

    
	// Final fragment color.
	return directLighting + ambientLighting;
}



//called by frag lighting shader
//input sun dir is directiuon rto sun
//texCoards are in range [0,1]
vec3 calculateLighting(
                      vec2 texCoards,
                      float depth,
                       mat4x4 inverseProj,
                       mat4x4 inverseView,
                      SampledPBRMaterial mat,
                        vec3 sunDir,
                        vec3 worldCamPos
                        ) {
        
    // re create frag wold pos
    // really good copmarison of ndc spaces - https://github.com/gpuweb/gpuweb/issues/416
    //for converting frag pos to world pos see: https://stackoverflow.com/questions/38938498/how-do-i-convert-gl-fragcoord-to-a-world-space-point-in-a-fragment-shader
    //anser by Nicol Bolas
    //and also see: https://stackoverflow.com/questions/32227283/getting-world-position-from-depth-buffer-value

    //z value may be wrong
    //vulkan ndc has z in depth range [0,1] - where 1 is the far plane and 0 is the near plane
    vec4 ndc = vec4(texCoards * 2.0f - 1.0f, depth, 1.0);

    //clip space has x and y in domain [-1,1]
    vec4 clipSpaceFragPos = ndc;


    vec4 viewSpaceFragPosition =
        inverseProj * clipSpaceFragPos;
    
    // perspective division
    viewSpaceFragPosition /= viewSpaceFragPosition.w;

    vec4 worldSpaceFragPosition =
        inverseView * viewSpaceFragPosition;

    Light sunLight;

    sunLight.type = 1;
    sunLight.direction = sunDir;
    sunLight.intensity = vec3(3);//vec3(4);//vec3(8);

    //mat.metallic = 1.0;
    //mat.roughness = 0.0;
    //mat.albedo = vec3(1);

    vec3 color = light(mat, sunLight, worldSpaceFragPosition.xyz, worldCamPos);
    
    return color;

    //cool normal packing info see here: https://aras-p.info/texts/CompactNormalStorage.html
}