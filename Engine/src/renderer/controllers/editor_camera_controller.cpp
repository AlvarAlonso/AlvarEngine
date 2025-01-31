#include "editor_camera_controller.hpp"

#include <renderer/core/camera.hpp>
#include <engine.hpp>
#include <core/logger.h>

#include <GLFW/glfw3.h>

namespace Alvar
{
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

    // TODO: Remove this and create an input/poll system.
    static double xMouseOld = 0.0;
    static double yMouseOld = 0.0;
    static bool MouseLocked = false;

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
            pCamera.lock()->Rotate(1.0f * m_Sensitivity * CEngine::Get()->GetDeltaTime(), 0.0f);
        }
        if (glfwGetKey(CEngine::Get()->GetWindow(), GLFW_KEY_Q))
        {
            pCamera.lock()->Rotate(-1.0f * m_Sensitivity * CEngine::Get()->GetDeltaTime(), 0.0f);
        }
        if (glfwGetKey(CEngine::Get()->GetWindow(), GLFW_KEY_ESCAPE))
        {
            MouseLocked = !MouseLocked;
        }
        
        if (MouseLocked)
        {
            SGSINFO("MOUSE LOCKED");
            double xMouse, yMouse;
            glfwGetCursorPos(CEngine::Get()->GetWindow(), &xMouse, &yMouse);

            const double xMouseDiff = xMouse - xMouseOld;
            const double yMouseDiff = yMouse - yMouseOld;

            pCamera.lock()->Rotate(
                static_cast<float>(xMouseDiff) * m_Sensitivity * CEngine::Get()->GetDeltaTime(), 
                -(static_cast<float>(yMouseDiff) * m_Sensitivity * CEngine::Get()->GetDeltaTime()));

            int WindowWidth, WindowHeight;
            glfwGetWindowSize(CEngine::Get()->GetWindow(), &WindowWidth, &WindowHeight);

            const int CenterX = static_cast<int>(floor(WindowWidth * 0.5f));
            const int CenterY = static_cast<int>(floor(WindowHeight * 0.5f));

            glfwSetCursorPos(CEngine::Get()->GetWindow(), CenterX, CenterY);
            xMouseOld = CenterX;
            yMouseOld = CenterY;
        }

        glm::normalize(Velocity);
        Velocity *= m_Speed * CEngine::Get()->GetDeltaTime();

        const glm::mat4 CameraRotation = pCamera.lock()->GetRotationMatrix();
        pCamera.lock()->m_Position += glm::vec3(CameraRotation * glm::vec4(Velocity, 0.f));
    }
}
