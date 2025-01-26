#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <memory>

const float DEFAULT_CAMERA_YAW = 0.0f;
const float DEFAULT_CAMERA_PITCH = 0.0f;

class CCamera : public std::enable_shared_from_this<CCamera>
{
public:
    CCamera(glm::vec3 aPosition = glm::vec3(0.5f, 0.5f, 0.5f), float aYaw = DEFAULT_CAMERA_YAW, float aPitch = DEFAULT_CAMERA_PITCH);

    glm::vec3 m_Position;

    float m_Yaw;
    float m_Pitch;

    glm::vec3 GetPosition() const { return m_Position; }
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjection() const;
    glm::mat4 GetRotationMatrix() const;

    void Rotate(float aXOfsset, float aYOffset, bool abConstraintPitch = true);
};