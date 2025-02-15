
#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 inPosition;
layout(location = 0) out vec2 outPosition;


//vec2 positions[3] = vec2[](
//    vec2(0.0, -0.5),
//    vec2(0.5, 0.5),
//    vec2(-0.5, 0.5)
//);
//
//vec3 colors[3] = vec3[](
//    vec3(1.0, 0.3, 0.0),
//    vec3(0.2, 1.0, 0.0),
//    vec3(0.0, 0.1, 1.0)
//);

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    outPosition = inPosition;
}
