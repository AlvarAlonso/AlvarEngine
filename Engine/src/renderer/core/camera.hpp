#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

const float DEFAULT_CAMERA_YAW = 0.0f;
const float DEFAULT_CAMERA_PITCH = 0.0f;
const float DEFAULT_CAMERA_SPEED = 0.01f;
const float DEFAULT_CAMERA_SENSITIVITY = 0.2f;

class CCamera
{
public:
    CCamera(glm::vec3 aPosition = glm::vec3(0.5f, 0.5f, 0.5f), float aYaw = DEFAULT_CAMERA_YAW, float aPitch = DEFAULT_CAMERA_PITCH, float aSpeed = DEFAULT_CAMERA_SPEED);

    glm::vec3 m_Position;

    float m_Yaw;
    float m_Pitch;
    float m_Speed;
    float m_Sensitivity;

    glm::vec3 GetPosition() const { return m_Position; }
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjection() const;

    void Update();

private:
    glm::mat4 GetRotationMatrix() const;
    void Rotate(float aXOfsset, float aYOffset, bool abConstraintPitch = true);
};