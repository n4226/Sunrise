#pragma once

// constants

#define PI 3.1415926535897932384626433832795
#define PI_f 3.1415926535897932384626433832795f
#define PI_2 1.57079632679489661923
#define PI_4 0.785398163397448309616


#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38
#define DBL_MAX 1.7976931348623158e+308
#define DBL_MIN 2.2250738585072014e-308

const float kInfinity = FLT_MAX;
//
//typedef float  xhalf;
//typedef vec2 xhalf2;
//typedef vec3 xhalf3;
//typedef vec4 xhalf4;
//typedef mat4x4 xhalf4x4;
//typedef mat3x3 xhalf3x3;


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