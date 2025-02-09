#pragma once

#include <core/IUpdatable.hpp>

#include <memory>

namespace Alvar
{
    class CCamera;

    // TODO: Expose this in ImGui.
    const float DEFAULT_CAMERA_SENSITIVITY = 0.001f;
    const float DEFAULT_CAMERA_SPEED = 0.05f;

    class CEditorCameraController : public IUpdatable, std::enable_shared_from_this<CEditorCameraController>
    {
    public:
        CEditorCameraController();

        float m_Sensitivity;
        float m_Speed;

        void SetCamera(std::weak_ptr<CCamera> apCamera);

        virtual void Update(float aDeltaTime) override;

    private:
        std::weak_ptr<CCamera> pCamera;
    };
}
