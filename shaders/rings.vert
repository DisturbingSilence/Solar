#version 460
layout(location = 0) in vec3 inPos;
layout(location = 0) out vec3 outUV;
layout(location = 0) uniform mat4 modelMatrix;
layout(binding = 0,std140) uniform globalUniforms
{
	uvec2 padding;
	uvec2 saturnRingsTexture;
	mat4 viewProj;
};
void main()
{
	gl_Position = viewProj * modelMatrix * vec4(inPos, 1.0);
	outUV = inPos;
}