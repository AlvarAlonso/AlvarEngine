#version 460

//gbuffers input
layout(set = 1, binding = 0) uniform sampler2D position;
layout(set = 1, binding = 1) uniform sampler2D normal;
layout(set = 1, binding = 2) uniform sampler2D albedo;

//output write
layout (location = 0) out vec4 outFragColor;

layout (location = 0) in vec2 uv;

void main() 
{	
	vec3 color = texture(albedo, uv).xyz;
	outFragColor = vec4(color, 1.0f);
	//outFragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}