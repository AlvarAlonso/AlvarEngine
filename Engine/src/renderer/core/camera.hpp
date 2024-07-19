#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

const float DEFAULT_CAMERA_YAW = 0.0f;
const float DEFAULT_CAMERA_PITCH = 0.0f;
const float DEFAULT_CAMERA_SPEED = 0.1f;
const float DEFAULT_CAMERA_SENSITIVITY = 0.2f;

class CCamera
{
public:
    CCamera(glm::vec3 aPosition = glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3 aUp = glm::vec3(0.0, 1.0f, 0.0f), float aYaw = DEFAULT_CAMERA_YAW, float aPitch = DEFAULT_CAMERA_PITCH);

    glm::vec3 m_Position;
    glm::vec3 m_Direction;
    glm::vec3 m_Up;
    glm::vec3 m_Right;

    float m_Yaw;
    float m_Pitch;
    float m_Speed;
    float m_Sensitivity;

    glm::vec3 GetPosition() const { return m_Position; }
    glm::mat4 GetView() const;
    glm::mat4 GetProjection() const;

private:
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetRotationMatrix() const;
    void UpdateCameraVectors();
    void Rotate(float aXOfsset, float aYOffset, bool abConstraintPitch = true);
};