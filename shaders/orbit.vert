#version 460

layout(location = 0) in vec3 inPos;
layout(location = 0) out vec3 outColor;

struct Orbit
{
    mat4 model;
    vec3 color;   
};
layout(binding = 0,std140) uniform globalUniforms
{
    uvec2 padding;
    uvec2 saturnRingsTexture;
    mat4 viewProj;
};

layout(binding = 1, std430) readonly restrict buffer orbitBuffer
{
    Orbit orbits[];
};

void main()
{
    gl_Position = viewProj * orbits[gl_InstanceID].model * vec4(inPos, 1.0);
    outColor = orbits[gl_InstanceID].color;
}