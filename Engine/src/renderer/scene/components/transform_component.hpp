#pragma once

#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Alvar
{
    struct sTransformComponent
    {
        sTransformComponent() :
            Translation({0.0f, 0.0f, 0.0f}),
            Rotation({0.0f, 0.0f, 0.0f}),
            Scale({1.0f, 1.0f, 1.0f}){}

        glm::vec3 Translation;
        glm::vec3 Rotation;
        glm::vec3 Scale;

        glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4(1.0f), Translation)
				* rotation
				* glm::scale(glm::mat4(1.0f), Scale);
		}
    };
}
