#include "editor_camera_controller.hpp"

#include <renderer/core/camera.hpp>
#include <engine.hpp>
#include <core/logger.h>
#include <core/input/input_module.hpp>

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
    static int32_t xMouseOld = 0.0;
    static int32_t yMouseOld = 0.0;
    static bool MouseLocked = false;

    void CEditorCameraController::Update(float aDeltaTime)
    {
        Input::CInputModule* pInput = CEngine::Get()->GetInputModule();

        glm::vec3 Velocity = glm::vec3(0.0f);

        if (pInput->GetButton(Key::Up))
        {
            Velocity.z = -1.0f;
        }
        if (pInput->GetButton(Key::Down))
        {
            Velocity.z = 1.0f;
        }
        if (pInput->GetButton(Key::Left))
        {
            Velocity.x = -1.0f;
        }
        if (pInput->GetButton(Key::Right))
        {
            Velocity.x = 1.0f;
        }
        if (pInput->GetButton(Key::E))
        {
            pCamera.lock()->Rotate(1.0f * m_Sensitivity * aDeltaTime, 0.0f);
        }
        if (pInput->GetButton(Key::Q))
        {
            pCamera.lock()->Rotate(-1.0f * m_Sensitivity * aDeltaTime, 0.0f);
        }

        MouseLocked = pInput->GetMouseButton(1) ? true : false;
        
        if (MouseLocked)
        {
            const glm::vec2 CurrentMousePos = pInput->GetMousePosition();

            const glm::vec2 MouseDiff = glm::vec2(CurrentMousePos.x - xMouseOld, CurrentMousePos.y - yMouseOld);

            const float MouseDiffLength = glm::length(MouseDiff);

            if (MouseDiffLength > 3.0f)
            {
                pCamera.lock()->Rotate(
                    -MouseDiff.x * m_Sensitivity * aDeltaTime, 
                    -MouseDiff.y * m_Sensitivity * aDeltaTime);
            }

            const glm::vec2 WindowCenter = CEngine::Get()->GetWindowCenter();
            CEngine::Get()->SetMouseCursor(WindowCenter);
            xMouseOld = WindowCenter.x;
            yMouseOld = WindowCenter.y;
        }

        glm::normalize(Velocity);
        Velocity *= m_Speed * aDeltaTime;

        const glm::mat4 CameraRotation = pCamera.lock()->GetRotationMatrix();
        const glm::vec3 CameraMovement = glm::vec3(CameraRotation * glm::vec4(Velocity, 0.f));
        pCamera.lock()->m_Position += CameraMovement;
    }
}
