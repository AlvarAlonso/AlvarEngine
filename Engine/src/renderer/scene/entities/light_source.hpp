#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp> 
#include <string>

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

    struct sLightSource
    {
        sLightSource();
        sLightSource(const sLightSource& aLightSource);
            
        std::string ID;    
        
        struct sProperties
        {
            sProperties();

            glm::mat4 Model;
            glm::vec3 TargetPosition;
            glm::vec3 Color;
            float MaxDist;
            float Intensity;
            float Radius;
            eLightType LightType;
        } Properties;
    };
}
