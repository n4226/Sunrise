#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_NV_viewport_array2: enable


layout(binding = 4) uniform UniformBufferObject {
    mat4 viewProjection[4];
} ubo;


layout(location = 0) in vec3 inPosition;

void main() {
    //there is no model matrix for lines
    gl_Position = ubo.viewProjection[0] * vec4(inPosition, 1.0);
}