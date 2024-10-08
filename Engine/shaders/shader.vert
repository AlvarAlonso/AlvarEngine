#version 460

layout(location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragWorldPos;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    mat4 viewproj;
    vec3 pos;
} ubo;

struct ObjectData {
    mat4 model;
};

layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer {
    ObjectData objects[];
} objectBuffer;

void main() {
    mat4 modelMatrix = objectBuffer.objects[gl_BaseInstance].model;
    vec4 worldPosition = modelMatrix * vec4(inPosition, 1.0);
    gl_Position = ubo.viewproj * worldPosition;
    fragColor = inColor;
    fragNormal = inNormal;
    fragTexCoord = inTexCoord;
    fragWorldPos = worldPosition.xyz;
}