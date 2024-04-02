#version 450

layout(location = 0) out vec4 outColor;

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragTexCoord;

layout (binding = 1) uniform sampler2D texSampler;

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
    int numLights;
}ubo;

layout (push_constant) uniform Push
{
    mat4 modelMatrix;
    mat4 normalMatrix;
}push;

layout (constant_id = 0) const int LIGHTING_MODEL = 0;
layout (constant_id = 1) const float PARAM_TOON_DESATURATION= 0.0f;

void main()
{
    
    // vec3 surfaceNormal = normalize(fragNormalWorld);

    // for (int i = 0; i < ubo.numLights; i++)
    // {
    //     PointLight light = ubo.pointLights[i];
    //     vec3 directionToLight = light.position.xyz - fragPosWorld;
    //     directionToLight = normalize(directionToLight);

    //     float intensity = dot(surfaceNormal, directionToLight);
    //     vec3 color;
    //     if (intensity > 0.98)
    //         color = light.color.xyz * 1.5;
    //     else if(intensity > 0.9)
    //         color = light.color.xyz * 1.0;
    //     else if(intensity > 0.5)
    //         color = light.color.xyz * 0.6;
    //     else if(intensity > 0.25)
    //         color = light.color.xyz * 0.4;
    //     else 
    //         color = light.color.xyz * 0.2;
    //     color = vec3(mix(color, vec3(dot(vec3(0.2126,0.7152, 0.0722),color)),0.5));
    //     outColor.rgb += color;
    // }
    outColor = vec4(1.f, 0.f, 0.f, 1.f);
       
}