#version 460

#extension GL_GOOGLE_include_directive : require
#include "light_functions.glsl"

layout(set = 0, binding = 1) uniform sampler2D texSampler;

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

layout(location = 0) out vec4 outColor;

void main() {
    outColor = materialConstants.Color;
}