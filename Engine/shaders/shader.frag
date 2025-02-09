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

struct LightData {
    vec3 Position;
    float MaxDist;
    vec3 TargetPosition;
    float Intensity;
    vec3 Color;
    float Radius;
    int LightType;
};

layout(push_constant) uniform PushConstants 
{
    int numLights;
} pushConstants;

layout(set = 3, binding = 0) readonly buffer LightsBuffer {
    LightData lights[];
} lightsBuffer;

layout(set = 2, binding = 1) uniform sampler2D albedoSampler;
layout(set = 2, binding = 2) uniform sampler2D metalRoughnessSampler;
layout(set = 2, binding = 3) uniform sampler2D emissiveSampler;
layout(set = 2, binding = 4) uniform sampler2D normalSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

void main() 
{
    vec3 color_texture = texture( albedoSampler, fragTexCoord ).xyz;
    float metal = texture(metalRoughnessSampler, fragTexCoord).z;
    float roughness = texture(metalRoughnessSampler, fragTexCoord).y;

	//calculate f0 reflection based on the color and metalness
	vec3 f0 = color_texture * metal + (vec3( 0.5 ) * ( 1.0 - metal ));

	//Normal has to be converted to clip space again
	vec3 N = texture( normalSampler, fragTexCoord ).xyz;
    N = normalize( N * 2.0 - 1.0 );
    
    vec3 totalLight = vec3(0.0);
    for (int i = 0; i < pushConstants.numLights; ++i)
    {
        LightData lightData = lightsBuffer.lights[i];
        // normalize the Light, Vision and Half vector and compute some dot products
        vec3 L = normalize( lightData.Position - fragWorldPos );
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
        vec3 direct = (kd + ks) * lightData.Color;

    	float att_factor = ComputeAttenuation(lightData.Position, fragWorldPos, lightData.MaxDist);

        vec3 currentLight = vec3(0.0);
        if (lightData.LightType == 1) // directional
        {
            currentLight = direct;
        }
        else if (lightData.LightType == 2) // Point
        {
            currentLight = direct * lightData.Intensity * att_factor;
        }
        else if (lightData.LightType == 3) // Spot
        {
            // Determine if it's inside light's cone.
            //direct *= SpotDirection(fragWorldPos);
		    //light = direct * intensity * att_factor;
        }
        else
        {
            // Invalid Type.
            // TODO: Find some way to debug this.
        }

        totalLight += currentLight;
    }

    outColor = vec4(totalLight * color_texture, 1.0);
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
}