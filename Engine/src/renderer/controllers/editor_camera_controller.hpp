#pragma once

#include <core/IUpdatable.hpp>

#include <memory>

class CCamera;

const float DEFAULT_CAMERA_SENSITIVITY = 0.2f;
const float DEFAULT_CAMERA_SPEED = 0.01f;

class CEditorCameraController : public IUpdatable, std::enable_shared_from_this<CEditorCameraController>
{
public:
    CEditorCameraController();

    float m_Sensitivity;
    float m_Speed;

    void SetCamera(std::weak_ptr<CCamera> apCamera);

    virtual void Update() override;

private:
    std::weak_ptr<CCamera> pCamera;
};