#include "camera.hpp"
#include "core/logger.h"
#include <engine.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <GLFW/glfw3.h>

CCamera::CCamera(glm::vec3 aPosition, float aYaw, float aPitch, float aSpeed) :
    m_Position(aPosition), m_Yaw(aYaw), m_Pitch(aPitch), m_Speed(aSpeed), m_Sensitivity(DEFAULT_CAMERA_SENSITIVITY)
{
}

void CCamera::Update()
{
    glm::vec3 Velocity = glm::vec3(0.0f);

    if (glfwGetKey(CEngine::Get()->GetWindow(), GLFW_KEY_UP))
    {
        Velocity.z = -1.0f;
    }
    if (glfwGetKey(CEngine::Get()->GetWindow(), GLFW_KEY_DOWN))
    {
        Velocity.z = 1.0f;
    }
    if (glfwGetKey(CEngine::Get()->GetWindow(), GLFW_KEY_LEFT))
    {
        Velocity.x = -1.0f;
    }
    if (glfwGetKey(CEngine::Get()->GetWindow(), GLFW_KEY_RIGHT))
    {
        Velocity.x = 1.0f;
    }
    if (glfwGetKey(CEngine::Get()->GetWindow(), GLFW_KEY_E))
    {
        Rotate(1.0f * 0.005f, 0.0f);
    }
    if (glfwGetKey(CEngine::Get()->GetWindow(), GLFW_KEY_Q))
    {
        Rotate(-1.0f * 0.005f, 0.0f);
    }

    glm::normalize(Velocity);
    Velocity *= m_Speed * CEngine::Get()->GetDeltaTime();

    const glm::mat4 CameraRotation = GetRotationMatrix();
    m_Position += glm::vec3(CameraRotation * glm::vec4(Velocity, 0.f));
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
    aXOfsset *= m_Sensitivity;
    aYOffset *= m_Sensitivity;

    m_Yaw -= aXOfsset;
    m_Pitch += aYOffset;

    if (abConstraintPitch) {
        if (m_Pitch > 89.0f)
            m_Pitch = 89.0f;
        if (m_Pitch < -89.0f)
            m_Pitch = -89.9f;
    }
}
