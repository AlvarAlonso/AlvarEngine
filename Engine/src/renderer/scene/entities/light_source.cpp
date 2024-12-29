#include "light_source.hpp"

sLightSource::sLightSource() :
    Model(glm::mat4(1.0f)), TargetPosition(glm::vec3(0.0f)), Color(glm::vec3(1.0f)), MaxDist(100.0f), 
    Intensity(1.0f), Radius(100.0f), LightType(eLightType::POINT)
{
}
