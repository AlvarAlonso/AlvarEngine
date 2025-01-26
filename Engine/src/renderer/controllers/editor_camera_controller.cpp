#include "editor_camera_controller.hpp"

#include <renderer/core/camera.hpp>
#include <engine.hpp>

#include <GLFW/glfw3.h>

CEditorCameraController::CEditorCameraController() :
    m_Sensitivity(DEFAULT_CAMERA_SENSITIVITY), m_Speed(DEFAULT_CAMERA_SPEED)
{
}

void CEditorCameraController::SetCamera(std::weak_ptr<CCamera> apCamera)
{
    if (apCamera.lock())
    {
        pCamera = apCamera;
    }
}

void CEditorCameraController::Update()
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
        pCamera.lock()->Rotate(1.0f * 0.005f * m_Sensitivity, 0.0f); // TODO: DeltaTime.
    }
    if (glfwGetKey(CEngine::Get()->GetWindow(), GLFW_KEY_Q))
    {
        pCamera.lock()->Rotate(-1.0f * 0.005f * m_Sensitivity, 0.0f);
    }

    glm::normalize(Velocity);
    Velocity *= m_Speed * CEngine::Get()->GetDeltaTime();

    const glm::mat4 CameraRotation = pCamera.lock()->GetRotationMatrix();
    pCamera.lock()->m_Position += glm::vec3(CameraRotation * glm::vec4(Velocity, 0.f));
}
