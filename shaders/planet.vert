#version 460
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 0) out vec2 outUV;
layout(location = 1) out flat uint outMaterial;
layout(binding = 0,std140) uniform globalUniforms
{
	uvec2 padding;
	uvec2 saturnRingsTexture;
	mat4 viewProj;
};
struct CelestialBody
{
	mat4 model;
	uint materialIdx;	
};
layout(binding = 1, std430) readonly restrict buffer celestialBuffer
{
	CelestialBody objects[];
};
void main() 
{	
	outUV = inUV;
	outMaterial = objects[gl_InstanceID].materialIdx;
	gl_Position = viewProj * objects[gl_InstanceID].model * vec4(inPos, 1.0);
}
