#version 460

#extension GL_GOOGLE_include_directive : require
#include "light_functions.glsl"

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    mat4 viewproj;
    vec3 pos;
} ubo;

layout(set = 2, binding = 0) uniform MaterialConstants {
    vec4 Color;
    float RoughnessFactor;
    float MetallicFactor;
    float TillingFactor;
    vec3 EmissiveFactor;
    bool bIsTransparent;
} materialConstants;

layout(set = 2, binding = 1) uniform sampler2D albedoSampler;
layout(set = 2, binding = 2) uniform sampler2D metalRoughnessSampler;
layout(set = 2, binding = 3) uniform sampler2D emissiveSampler;
layout(set = 2, binding = 4) uniform sampler2D normalSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

// TODO: Pass lights into the shader.

void main() {
    vec3 color_texture = texture( albedoSampler, fragTexCoord ).xyz;
    float metal = texture(metalRoughnessSampler, fragTexCoord).z;
    float roughness = texture(metalRoughnessSampler, fragTexCoord).y;

	//calculate f0 reflection based on the color and metalness
	vec3 f0 = color_texture * metal + (vec3( 0.5 ) * ( 1.0 - metal ));

	//Normal has to be converted to clip space again
	vec3 N = texture( normalSampler, fragTexCoord ).xyz;
    N = normalize( N * 2.0 - 1.0 );
    
    // normalize the Light, Vision and Half vector and compute some dot products
	vec3 L = normalize( light_position - fragWorldPos );
	vec3 V = normalize( ubo.pos - fragWorldPos );
	vec3 H = normalize( L + V );
	float NdotL = clamp( dot( N, L ), 0.0, 1.0 );
	float NdotV = clamp( dot( N, V ), 0.0, 1.0 );
	float NdotH = clamp( dot( N, H ), 0.0, 1.0 );
	float LdotH = clamp( dot( L, H ), 0.0, 1.0 );

    // calulate the specular and difusse
	vec3 ks = SpecularBRDF( roughness, f0, NdotH, NdotV, NdotL, LdotH );
	vec3 diffuse = ( 1.0 - metal ) * color_texture;	//the most metalness the less diffuse color
	vec3 kd = diffuse * NdotL;
	vec3 direct = kd + ks;

    outColor = vec4(direct * color_texture, 1.0);
}