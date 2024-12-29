#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
 
enum class eLightType : uint8_t
{
    NONE = 0,
    DIRECTIONAL,
    POINT,
    SPOT,
    NUM
};

struct sLightSource
{
    sLightSource();

    glm::mat4 Model;
    glm::vec3 TargetPosition;
    glm::vec3 Color;
    float MaxDist;
    float Intensity;
    float Radius;
    eLightType LightType;
};