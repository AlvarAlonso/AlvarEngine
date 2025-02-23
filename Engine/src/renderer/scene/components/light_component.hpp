#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace Alvar
{   
    enum class eLightType : int32_t
    {
        NONE = 0,
        DIRECTIONAL,
        POINT,
        SPOT,
        NUM
    };

    struct sLightComponent
    {
        sLightComponent() : 
            TargetPosition({0.0f, 0.0f, 0.0f}),
            Color({1.0f, 1.0f, 1.0f}),
            MaxDist(10.0f),
            Intensity(10.0f),
            Radius(10.0f),
            SpotCosine(0.5f),
            LightType(eLightType::POINT){}

        glm::vec3 TargetPosition;
        glm::vec3 Color;
        float MaxDist;
        float Intensity;
        float Radius;
        float SpotCosine;
        eLightType LightType;
    };
}
