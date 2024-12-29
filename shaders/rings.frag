#version 460
#extension GL_ARB_bindless_texture : require

layout(location = 0) in vec3 inUV;

layout(location = 0) out vec4 FragColor;

layout(binding = 0,std140) uniform globalUniforms
{
	uvec2 padding;
	uvec2 saturnRingsTexture;
	mat4 viewProj;
};

void main() 
{
	// distance from center of Saturn to edge of rings
    float d = distance(inUV, vec3(0)); 
    // based on distance from center get color from color map
    vec4 color = texture(sampler2D(saturnRingsTexture), vec2(d,0));
    if(d < 0.2 || d > 0.5 ) color.w = 0; // if too far away or too close - make transparent to look like a ring
    FragColor = color;
  
}