#include "camera.hpp"
#include "core/logger.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

CCamera::CCamera(glm::vec3 aPosition, glm::vec3 aUp, float aYaw, float aPitch) :
    m_Position(aPosition), m_Direction(glm::vec3(0.0f, 0.0f, -1.0f)), m_Up(aUp), m_Right(), 
    m_Yaw(aYaw), m_Pitch(aPitch), m_Speed(DEFAULT_CAMERA_SPEED), m_Sensitivity(DEFAULT_CAMERA_SENSITIVITY)
{
    UpdateCameraVectors();
}

glm::mat4 CCamera::GetView() const
{
    // SGSINFO("Camera Position: %f, %f, %f", m_Position.x, m_Position.y, m_Position.z);
    // SGSINFO("Camera Rotation: Yaw %f, Pitch %f", m_Yaw, m_Pitch);
    return GetViewMatrix();
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

void CCamera::HandleInput(WPARAM aInput)
{
    glm::mat4 cameraRotation = GetRotationMatrix();
    glm::vec3 velocity;

    if (aInput == VK_LEFT)
    {
        velocity.y = -1.0f;
    }
    else if (aInput == VK_RIGHT)
    {
        velocity.y = 1.0f;
    }
    else if (aInput == VK_UP)
    {
        velocity.z = 1.0f;
    }
    else if (aInput == VK_DOWN)
    {
        velocity.z = -1.0f;
    }
    else if (aInput == 'W')
    {
        velocity.x = -1.0f;
    }
    else if (aInput == 'S')
    {
        velocity.x = 1.0f;
    }
    else if (aInput == 'Q')
    {
        Rotate(-1.0f, 0.0f);
    }
    else if (aInput == 'E')
    {
        Rotate(1.0f, 0.0f);
    }

    m_Position += glm::vec3(cameraRotation * glm::vec4(velocity * 0.5f, 0.f));
}

void CCamera::UpdateCameraVectors()
{
    // glm::vec3 Front;
    // Front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    // Front.y = sin(glm::radians(m_Pitch));
    // Front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

    // SGSINFO("Front: %f, %f, %f.", Front.x, Front.y, Front.z);

    // m_Up = glm::vec3(0.0f, 0.0f, 1.0f);
    // m_Direction = glm::normalize(Front);
    // m_Right = glm::normalize(glm::cross(m_Direction, m_Up));
    // //m_Right = glm::normalize(glm::cross(m_Direction, glm::vec3(0, 0, 1)));
    // SGSINFO("m_Right: %f, %f, %f.", m_Right.x, m_Right.y, m_Right.z);
    // //m_Up = glm::normalize(glm::cross(m_Right, m_Direction));
    // SGSINFO("m_Up: %f, %f, %f.", m_Up.x, m_Up.y, m_Up.z);
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

    UpdateCameraVectors();
}
