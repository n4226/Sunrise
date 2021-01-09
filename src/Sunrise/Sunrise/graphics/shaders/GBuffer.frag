
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable



// Lighting math lots of code from ----- 

// this was implemented by following sevral totorials:
// Alan Zucconi: Volumetric Atmospheric Scattering - https://www.alanzucconi.com/?p=7374
// nvidea : ///

//

#define PI 3.1415926535897932384626433832795
#define PI_2 1.57079632679489661923
#define PI_4 0.785398163397448309616


#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38
#define DBL_MAX 1.7976931348623158e+308
#define DBL_MIN 2.2250738585072014e-308

const float kInfinity = FLT_MAX;


/////// default constants

//#define earthRadius 6378137
//#define atmosphereRadius 6378137 + 60000

//// atm constants
//// thease are hight scalese
//#define Hr float(7994)
//#define Hm float(1200)
//
//// some function of (lambda) - constant for each type of scatterings
#define betaR vec3(3.8e-6f, 13.5e-6f, 33.1e-6f)
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



layout(binding = 4) uniform UniformBufferObject {
// global uniforms
    mat4 viewProjection;

    // post uniforms
    mat4 invertedViewMat;
    mat4 viewMat;
    mat4 projMat;

    vec4 earthCenter;
    vec4 sunDir;
    vec4 camFloatedGloabelPos;
    ivec2 renderTargetSize;
} ubo;




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



bool raySphereIntersect(vec3 orig, vec3 dir, float radius, out float t0,out float t1)
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
    uint numSamples = 12;//8;//16;
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
                                          vec2 textPosition,
                                          vec3 camPos, // without floating origin  // world // in geo coordinates
                                          mat4x4 ViewMatrix,
                                          vec3 sunDirection
) {

    // calculate origin(pos) and direction(dir)

    vec3 orig = camPos;
    
//#warning remmber that this fov and other camera info is not checked in any way to e what is on the CPU so be carful

// swift way

//    float aspectRatio = textureSize.x / float(textureSize.y);
//    float fov = 60;
//    float angle = tan(fov * PI / 180 * 0.5f);
//    
//    //
//    float rayx = (2 * textPosition.x - 1) * aspectRatio * angle;
//    float rayy = (1 - textPosition.y * 2) * angle;
//    
//    vec3 dir = vec3(rayx, rayy, -1);
//    //    dir = normalize(dir);
//    dir = normalize((mat4(ViewMatrix) * vec4(dir,0)).xyz);
//


    vec4 reverseVec;

    /* inverse perspective projection */
    reverseVec = vec4(textPosition.xy * 2 - 1, 0.0, 1.0);
    reverseVec = inverse(ubo.projMat) * reverseVec;

    /* inverse modelview, without translation */
    reverseVec.w = 0.0;
    reverseVec = ubo.invertedViewMat * reverseVec;
  
    /* send */
    vec3 dir = vec3(reverseVec);


    // 

    float t0, t1, tMax = kInfinity;
    // if the view ray intersects earth set the max to be the distance/time till the surface
    if (raySphereIntersect(orig, dir, earthRadius, t0, t1) && t1 > 0) {
            tMax = max(0, t0);
    }

    return computeIncidentLight(orig,dir,0,tMax,sunDirection);

    //return dir;//vec3(0,0,0.8) * (textPosition.x * 1);

}


// PBR MATH



float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
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

//vec3 calculateLighting(
//                      vec2 texCoards,
//                      float depth,
//                      SampledPBRMaterial mat,
//                        ) {
//        
//}


// end Lighting math






layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput GBuffer_Albedo_Metallic;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput GBuffer_Normal_Roughness;
layout (input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput GBuffer_AO;
layout (input_attachment_index = 3, set = 0, binding = 3) uniform subpassInput GBuffer_Depth;

float remap(float value, float low1,float high1,float low2,float high2) {
    return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
}

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 inPos;

void main() {
    
    vec4 albedo_metallic =   subpassLoad(GBuffer_Albedo_Metallic);
    vec4 normal_sroughness = subpassLoad(GBuffer_Normal_Roughness);
    float ao =               subpassLoad(GBuffer_AO).x;
    float depth =            subpassLoad(GBuffer_Depth).x; 

    //albedo_metallic = normal_sroughness;

    //depth = remap(depth,0.9993,1,0,1);
    //albedo_metallic.xyz = vec3(depth);


    if (normal_sroughness.w == 0) {
        albedo_metallic.xyz = calculatePostAtmosphereicScatering(ubo.renderTargetSize,inPos.xy * 0.5 + 0.5,ubo.camFloatedGloabelPos.xyz - ubo.earthCenter.xyz,ubo.viewMat,ubo.sunDir.xyz);
        albedo_metallic.w = 1;
    }

    outColor = vec4(albedo_metallic.xyz, 1);
}