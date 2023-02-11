#ifndef atmScat_h
#define atmScat_h

#include "../base.h"


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
//foggy but doesnt work
//#define betaM vec3(21e-6f * 10.0f)

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
    vec3 sunDirection,
    vec3 originalFragColor, // the original light at tmax - e.g this is the original fragment color - if no fragment this is zero
    bool highPrecision
    )
{

    float t0, t1;
    if (!raySphereIntersect(orig, dir, atmosphereRadius, t0, t1) || t1 < 0) return vec3(0);
    if (t0 > tmin && t0 > 0) tmin = t0;
    if (t1 < tmax) tmax = t1;

    uint numSamples, numSamplesLight;

//regular
    // if (highPrecision) {
    //     numSamples = 10;
    //     numSamplesLight = 5;
    // } else {
    //     numSamples = 2;
    //     numSamplesLight = 1;
    // }

//faster
    if (highPrecision) {
    numSamples = 8;
    numSamplesLight = 4;
    } else {
        numSamples = 2;
        numSamplesLight = 1;
    }

    // uint numSamples = 12;//12;//16;
    // uint numSamplesLight = 6; //6;// 8;



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


    float sunIntensity = 20;//20;//22;
    vec3 atmColor = (sumR * betaR * phaseR + sumM * betaM * phaseM) * sunIntensity;

    //Ray Color = Object Color * (1 - Transmittance) + Atmosphere Color
    vec3 rayColor = atmColor + originalFragColor * (1 - (atmColor * 0));//originalFragColor * (1 - (atmColor)) + atmColor * (originalFragColor);
    return rayColor;
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



//new notes:
// this is a really good source: https://provod.works/blog/2018-06-11-scattering/
//has way to quickely fake clouds and blend sky with scene
//another good source - https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/simulating-sky/simulating-colors-of-the-sky

//a really good shader example: https://www.shadertoy.com/view/WlSSzK

vec3 calculatePostAtmosphereicScatering(
    vec2 ndc,  // x and y of this fragment in the texture // in [0,1]
    vec3 camPos, // in world space i.e floated coordinates such that it is in same coordinate space as the fragement 
    mat4x4 invertedProjMat,
    mat4x4 invertedViewMat,
    vec3 sunDirection,
    vec3 earthCenter,
    vec3 originalFragColor,
    float depth
) {
    vec4 clipSpaceFragPos = vec4(ndc, depth, 1.0);

    vec4 viewSpaceFragPosition =
        invertedProjMat * clipSpaceFragPos;
    
    // perspective division
    viewSpaceFragPosition /= viewSpaceFragPosition.w;

    vec4 worldSpaceFragPosition =
        invertedViewMat * viewSpaceFragPosition;

    vec3 dirTemp = worldSpaceFragPosition.xyz - camPos;
    vec3 dir =  normalize(dirTemp);
    vec3 orig = camPos - earthCenter;

    // return earthCenter;

    //#temp to visualize normals
    //return vec3(1) - dir;

    // 

    float t0, t1, tMax = kInfinity;
    bool highPrecision = true;

    if (depth < 1) {
        tMax = viewSpaceFragPosition.z;
        // highPrecision = false;
    }

    //if the view ray intersects earth set the max to be the distance/time till the surface
    //TODO: un comment this out;;;;;;;
    if (raySphereIntersect(orig, dir, earthRadius, t0, t1) && t1 > 0) {
        tMax = min(tMax,max(0, t0));
        // highPrecision = false;
    }

    // float at0, at1; // - maybe problem with atmosphere being only visible inside planet is because t1 is less then zero in other func?
    // if (raySphereIntersect(orig,dir,earthRadius,at0, at1) && at1 > 0) {
    //     return vec3(0.5,0.4,0.1);
    // }
    // else return vec3(0,0,0.3);

    const float epsilon = 0.000001;
    float tmin = 0 + epsilon;

    return computeIncidentLight(orig, dir, tmin, tMax - epsilon, sunDirection,originalFragColor, highPrecision);

}

vec3 calculatePostAtmosphereicScatering(
    vec3 orig,
    vec3 dir,
    vec3 sunDirection,
    vec3 originalFragColor) {

    float t0, t1, tMax = kInfinity;
    
    if (raySphereIntersect(orig, dir, earthRadius, t0, t1) && t1 > 0) {
        tMax = max(0, t0);
    }

    const float epsilon = 0.000001;
    float tmin = 0 + epsilon;

    return computeIncidentLight(orig, dir, tmin, tMax - epsilon, sunDirection,originalFragColor, true);
}
#endif //atmScat_h