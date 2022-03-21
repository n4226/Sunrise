#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable
#extension GL_GOOGLE_include_directive : enable


//#include "headers/lighting/atmScat.h"
#include "headers/lighting/pbr.h"


//TODO: move this back to binding 4 when depth buffer added back
layout(binding = 4) uniform UniformBufferObject {
// global uniforms
    mat4 viewProjection;

    // post uniforms
    mat4 invertedViewMat;
    mat4 invertedProjMat;
    mat4 viewMat;
    mat4 projMat;

    vec4 earthCenter;
    vec4 sunDir;
    vec4 camFloatedGloabelPos;
    ivec2 renderTargetSize;
} ubo;



// Lighting math lots of code from ----- 

// this was implemented by following sevral totorials:
// Alan Zucconi: Volumetric Atmospheric Scattering - https://www.alanzucconi.com/?p=7374
// nvidea : ///

//


/////// default constants

//#define earthRadius 6378137
//#define atmosphereRadius 6378137 + 60000

//// atm constants
//// thease are hight scalese
//#define Hr float(7994)
//#define Hm float(1200)
//
//// some function of (lambda) - constant for each type of scatterings
// defualts:
// #define betaR vec3(3.8e-6f, 13.5e-6f, 33.1e-6f)
// #define betaM vec3(21e-6f)

//from: https://www.shadertoy.com/view/wlBXWK
#define betaR vec3(5.5e-6, 13.0e-6, 22.4e-6)
#define betaM vec3(21e-6f)

//

#define earthRadius 6378137
#define atmosphereRadius 6378137 + 60000

// atm constants
// thease are hight scalese
#define Hr float(8e3)
#define Hm float(1.2e3)

// some function of (lambda) - constant for each type of scatterings
//#define betaR vec3(5.5e-6, 13.0e-6, 22.4e-6)
//#define betaM vec3(21e-6f)


bool solveQuadratic(float a, float b, float c, out float x1, out float x2)
{
    if (b == 0) {
        // Handle special case where the the two vector ray.dir and V are perpendicular
        // with V = ray.orig - sphere.centre
        if (a == 0) return false;
        x1 = 0; x2 = sqrt(-c / a);
        return true;
    }
    float discr = b * b - 4 * a * c;

    if (discr < 0) return false;

    float q = (b < 0.f) ? -0.5f * (b - sqrt(discr)) : -0.5f * (b + sqrt(discr));
    x1 = q / a;
    x2 = c / q;

    return true;
}



bool raySphereIntersect(vec3 orig, vec3 dir, float radius, out float t0, out float t1)
{
    // They ray dir is normalized so A = 1
    float A = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
    float B = 2 * (dir.x * orig.x + dir.y * orig.y + dir.z * orig.z);
    float C = orig.x * orig.x + orig.y * orig.y + orig.z * orig.z - radius * radius;

    if (!solveQuadratic(A, B, C, t0, t1)) {
        return false;
    }

    if (t0 > t1) {
        float temp = t0;
        t0 = t1;
        t1 = temp;
    }

    return true;
}



vec3 computeIncidentLight(
    vec3 orig,
    vec3 dir,
    float tmin,
    float tmax,
    vec3 sunDirection)
{

    float t0, t1;
    if (!raySphereIntersect(orig, dir, atmosphereRadius, t0, t1) || t1 < 0) return vec3(0);
    if (t0 > tmin && t0 > 0) tmin = t0;
    if (t1 < tmax) tmax = t1;
    uint numSamples = 12;//12;//16;
    uint numSamplesLight = 6;// 8;
    float segmentLength = (tmax - tmin) / numSamples;
    float tCurrent = tmin;
    vec3 sumR = vec3(0); // mie and rayleigh contribution
    vec3 sumM = vec3(0); // mie and rayleigh contribution
    float opticalDepthR = 0, opticalDepthM = 0;
    float mu = dot(dir, sunDirection); // mu in the paper which is the cosine of the angle between the sun direction and the ray direction
    float phaseR = 3.f / (16.f * PI) * (1 + mu * mu);
    float g = 0.76f;
    float phaseM = 3.f / (8.f * PI) * ((1.f - g * g) * (1.f + mu * mu)) / ((2.f + g * g) * pow(1.f + g * g - 2.f * g * mu, 1.5f));



    for (uint i = 0; i < numSamples; ++i) {
        vec3 samplePosition = orig + (tCurrent + segmentLength * 0.5f) * dir;
        float height = length(samplePosition) - earthRadius;
        // compute optical depth for light
        float hr = exp(-height / Hr) * segmentLength;
        float hm = exp(-height / Hm) * segmentLength;
        opticalDepthR += hr;
        opticalDepthM += hm;
        // light optical depth
        float t0Light, t1Light;
        raySphereIntersect(samplePosition, sunDirection, atmosphereRadius, t0Light, t1Light);
        // t1Light = time along sun ray to leaving atmosphere
        float segmentLengthLight = t1Light / numSamplesLight, tCurrentLight = 0;
        float opticalDepthLightR = 0, opticalDepthLightM = 0;
        uint j;
        for (j = 0; j < numSamplesLight; ++j) {
            // samplePosition = current point on view ray
            // tCurrentLight = the current time along sun ray; ([t0Light,t1Light] )- i believe'
            // segmentLengthLight = the length of each section of the sun ray in this j loop
            // 0.5 - because the math is done for the middle pint of each line segment of the ray calclulated in each loop - so sun dir is scalled to the middle of the line segment
            // samplePositionLight = the point in the middle of the line semgnet calculated in this loop from the sun dir ray and described in the above comments.
            // the samplePositionLight vecotor represtents the radius plus hight in the diagr
            vec3 samplePositionLight = samplePosition + (tCurrentLight + segmentLengthLight * 0.5f) * sunDirection;
            float heightLight = length(samplePositionLight) - earthRadius;
            if (heightLight < 0) break;
            opticalDepthLightR += exp(-heightLight / Hr) * segmentLengthLight;
            opticalDepthLightM += exp(-heightLight / Hm) * segmentLengthLight;
            tCurrentLight += segmentLengthLight;
        }
        if (j == numSamplesLight) {
            vec3 tau = betaR * (opticalDepthR + opticalDepthLightR) + betaM * 1.1f * (opticalDepthM + opticalDepthLightM);
            vec3 attenuation = exp(-tau);
            sumR += attenuation * hr;
            sumM += attenuation * hm;
        }
        tCurrent += segmentLength;
    }



    return (sumR * betaR * phaseR + sumM * betaM * phaseM) * 22;
}



//struct Ray {
//    vec3 O; // Origin
//    vec3 V; // Direction vector
//};
//
//// Notes: GLUP.viewport = [x0,y0,width,height]
//// clip-space coordinates are in [-1,1] (not [0,1]) !
//
//// Computes the ray that passes through the current fragment
//// The ray is in world space.
//Ray glup_primary_ray() {
//    vec4 near = vec4(
//    2.0 * ( (gl_FragCoord.x - 0) - 0.5),
//    2.0 * ( (gl_FragCoord.y - 0) - 0.5),
//        0.0,
//        1.0
//    );
//    near = GLUP.inverse_modelviewprojection_matrix * near ;
//    vec4 far = near + GLUP.inverse_modelviewprojection_matrix[2] ;
//    near.xyz /= near.w ;
//    far.xyz /= far.w ;
//    return Ray(near.xyz, far.xyz-near.xyz) ;
//}


vec3 calculatePostAtmosphereicScatering(
    ivec2 textureSize,
    vec2 ndc,  // x and y of this fragment in the texture // in [0,1]
    vec3 camPos, // without floating origin  // world // in geo coordinates
    mat4x4 ViewMatrix,
    vec3 sunDirection
) {

    vec4 clipSFPos = vec4(ndc, 1.0, 1.0);

    vec4 viewSFPosition = 
        ubo.invertedProjMat * ubo.invertedViewMat * clipSFPos;

    viewSFPosition /= viewSFPosition.w;

    // return vec3(viewSFPosition.xyz);


//return camPos;//vec3(ndc,0);




   // calculate origin(pos) and direction(dir)
        vec3 orig = camPos;

        //vec4 clipSpaceFragPos = vec4((ndc * 2) - 1, 0.5f, 1.0);

    // // dont think i need this but i dont know;
    // clipSpaceFragPos.y = -clipSpaceFragPos.y;


    //  vec4 viewSpaceFragPosition =
    //     ubo.invertedProjMat * clipSpaceFragPos;
    
    // // perspective division
    // viewSpaceFragPosition /= viewSpaceFragPosition.w;

    // vec4 worldSpaceFragPosition =
    //     ubo.invertedViewMat * viewSpaceFragPosition;




//    
//
//
//    //#warning remmber that this fov and other camera info is not checked in any way to e what is on the CPU so be carful
//
//    // swift way
//
//    //    float aspectRatio = textureSize.x / float(textureSize.y);
//    //    float fov = 60;
//    //    float angle = tan(fov * PI / 180 * 0.5f);
//    //    
//    //    //
//    //    float rayx = (2 * textPosition.x - 1) * aspectRatio * angle;
//    //    float rayy = (1 - textPosition.y * 2) * angle;
//    //    
//    //    vec3 dir = vec3(rayx, rayy, -1);
//    //    //    dir = normalize(dir);
//    //    dir = normalize((mat4(ViewMatrix) * vec4(dir,0)).xyz);
//    //
//
//
//    vec4 reverseVec;
//
//    //TODO: this whole file is a complete mess but it apears to be less lickely for the sky to fall
//
//    /* inverse perspective projection */
//
//    // in [-1,1] for x and y
//    vec2 normalizedUvs = textPosition.xy * 2 - 1;
//
//    // see: https://veldrid.dev/articles/backend-differences.html
//    // Vulkan: [-1,1][-1,1][0,1] NOTE: Vulkan's clip space Y axis is inverted compared to other API's.
//
//    // the view direction in NDC space ([-1.1], [-1,1], [0,1] -z (0z is near plane))
//    vec3 viewDirNDC = normalize(vec3(normalizedUvs,1));
//
//    vec3 viewDirEye = (inverse(ubo.projMat) * vec4(viewDirNDC,1)).xyz;
//    // view direection in world space
//    vec3 viewDerWorld = (ubo.invertedViewMat * vec4(viewDirEye,0)).xyz;
//
////
////    reverseVec = vec4(normalizedUvs, 0.0, 1.0);
////    reverseVec = inverse(ubo.projMat) * reverseVec;
////
////    /* inverse modelview, without translation */
////    reverseVec.w = 0.0;
////    reverseVec = ubo.invertedViewMat * reverseVec;
////
//
//
//    
//
//
//
//    /* send */
//    vec3 dir = viewDerWorld;//vec3(reverseVec);
//

    vec4 clipSpaceFragPos = vec4(ndc, 0.5f, 1.0);

    // dont think i need this but i dont know;
    //clipSpaceFragPos.y = -clipSpaceFragPos.y;


     vec4 viewSpaceFragPosition =
        ubo.invertedProjMat * clipSpaceFragPos;
    
    // perspective division
    viewSpaceFragPosition /= viewSpaceFragPosition.w;

    vec4 worldSpaceFragPosition =
        ubo.invertedViewMat * viewSpaceFragPosition;

    vec3 dirTemp = worldSpaceFragPosition.xyz - ubo.camFloatedGloabelPos.xyz;
    vec3 dir =  normalize(dirTemp);

    //#temp to visualize normals
    //return vec3(1) - dir;

    // 

    float t0, t1, tMax = kInfinity;
    //if the view ray intersects earth set the max to be the distance/time till the surface
    //TODO: un comment this out;;;;;;;
    // if (raySphereIntersect(orig, dir, earthRadius, t0, t1) && t1 > 0) {
    //     tMax = max(0, t0);
    // }

    // float at0, at1; // - maybe problem with atmosphere being only visible inside planet is because t1 is less then zero in other func?
    // if (raySphereIntersect(orig,dir,earthRadius,at0, at1) && at1 > 0) {
    //     return vec3(0.5,0.4,0.1);
    // }
    // else return vec3(0,0,0.3);

    const float epsilon = 0.000001;
    float tmin = 0 + epsilon;

    return computeIncidentLight(orig, dir, tmin, tMax - epsilon, sunDirection);

    //return dir;//vec3(0,0,0.8) * (textPosition.x * 1);

}






// end Lighting math





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


    vec3 color;

    //TODO: remove this to render the actual scene
    if (depth == 1) {
        color.xyz = calculatePostAtmosphereicScatering(ubo.renderTargetSize,inPos.xy,ubo.camFloatedGloabelPos.xyz - ubo.earthCenter.xyz,ubo.viewMat,ubo.sunDir.xyz);
        // color *= ubo.sunDir.xyz;
        color.xyz = vec3(0,0.2,0.4) * 1; 
        //albedo_metallic.w = 1;
    }
    else {
        SampledPBRMaterial mat;
        mat.albedo = albedo_metallic.xyz;
        mat.worldSpaceNormals = normal_sroughness.xyz;
        mat.metallic = albedo_metallic.w;
        mat.ao = ao.x;
        mat.roughness = normal_sroughness.w;

        color = calculateLighting(uvs,depth,ubo.invertedViewMat,ubo.invertedProjMat,mat,ubo.sunDir.xyz,ubo.camFloatedGloabelPos.xyz);
        
        //float brightness = dot(normalize(normal_sroughness.xyz),normalize(ubo.sunDir.xyz));
        //color = vec3(normal_sroughness.xyz) * max(brightness,0);
    }

 // todo add ACES tone mapping in a better place such as post pass
    color = ACESFitted(color);

    outColor = vec4(color, 1);
}