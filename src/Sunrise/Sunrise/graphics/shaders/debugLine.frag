#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable


layout( push_constant ) uniform DrawPushData {
  vec4 color;
} drawData;


layout (binding = 3) uniform sampler2D GBuffer_Depth;


layout(binding = 4) uniform UniformBufferObject {
// global uniforms
    mat4 viewProjection[4];

    // post uniforms
    mat4 invertedViewMat[4];
    mat4 invertedProjMat[4];
    mat4 viewMat[4];
    mat4 projMat[4];
    vec4 camFloatedGloabelPos[4];

    vec4 earthCenter;
    vec4 sunDir;
    ivec2 renderTargetSize;
} ubo;


layout(location = 0) out vec4 outColor;

void main() {

    vec2 uv = gl_FragCoord.xy / ubo.renderTargetSize.xy;
    float depth = gl_FragCoord.z;// / gl_FragCoord.w;
    float measuredDepth =  texture(GBuffer_Depth,uv).x;

    if (depth > measuredDepth) {
       discard;
    }
    outColor = drawData.color;
}