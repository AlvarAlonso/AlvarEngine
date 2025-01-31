#include "camera.hpp"
#include "core/logger.h"
#include <engine.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Alvar
{
    CCamera::CCamera(glm::vec3 aPosition, float aYaw, float aPitch) :
        m_Position(aPosition), m_Yaw(aYaw), m_Pitch(aPitch)
    {
    }

    glm::mat4 CCamera::GetViewMatrix() const
    {
        glm::mat4 cameraTranslation = glm::translate(glm::mat4(1.f), m_Position);
        glm::mat4 cameraRotation = GetRotationMatrix();
        return glm::inverse(cameraTranslation * cameraRotation);
    }

    glm::mat4 CCamera::GetRotationMatrix() const
    {
        glm::quat pitchRotation = glm::angleAxis(m_Pitch, glm::vec3 { 1.f, 0.f, 0.f });
        glm::quat yawRotation = glm::angleAxis(m_Yaw, glm::vec3 { 0.f, -1.f, 0.f });

        return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);
    }

    glm::mat4 CCamera::GetProjection() const
    {
        // TODO: Implement this.
        return glm::mat4(1);
    }

    void CCamera::Rotate(float aXOfsset, float aYOffset, bool abConstraintPitch)
    {
        m_Yaw -= aXOfsset;
        m_Pitch += aYOffset;

        if (abConstraintPitch) {
            if (m_Pitch > 89.0f)
                m_Pitch = 89.0f;
            if (m_Pitch < -89.0f)
                m_Pitch = -89.9f;
        }
    }
}
