#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;


struct PointLight
{
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo{
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    float outlineWidth;
    int numLights;
}ubo;

layout (push_constant) uniform Push
{
    mat4 modelMatrix;
    mat4 normalMatrix;
}push;
void main()
{
    vec3 fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
    vec4 pos = push.modelMatrix * vec4(position.xyz + fragNormalWorld * ubo.outlineWidth, 1.f);
    gl_Position = ubo.projection * ubo.view * pos;
}