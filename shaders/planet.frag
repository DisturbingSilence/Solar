#version 460
#extension GL_ARB_bindless_texture : require
layout(location = 0) in vec2 inUV;
layout(location = 1) in flat uint inMaterialIdx;
layout(location = 0) out vec4 FragColor;
#define HAS_BASE_COLOR_TEXTURE 1
#define CHECK_FLAG_BIT(value,flag) ((value & flag) == flag)
struct ShaderMaterial
{
	uvec2 baseColorTexture;
	uint flags;
};
layout(binding = 0, std430) readonly restrict buffer materialBuffer 
{
	ShaderMaterial materials[];
};
void main() 
{
	ShaderMaterial mat = materials[inMaterialIdx];
	vec4 baseColor = vec4(1.0);
	if (CHECK_FLAG_BIT(mat.flags,HAS_BASE_COLOR_TEXTURE)) 
	{
		baseColor *= texture(sampler2D(mat.baseColorTexture), inUV);
	}
	FragColor = baseColor;
}
