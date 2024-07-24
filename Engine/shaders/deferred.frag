#version 460

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 texCoord;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outFragColor;

void main() 
{	
	vec3 color = vec3(1.0, 0.0, 0.0);
	outPosition = vec4(inPosition, 1.0f);
	outNormal = vec4(inNormal, 1.0f);
	outFragColor = vec4(color, 1.0f);
}