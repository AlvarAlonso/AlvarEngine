#pragma once

#include "Vulkan/vulkan_backend.hpp"
#include "scene.hpp"
#include "core/camera.hpp"
#include <core/IModule.hpp>

#include <memory>
#include <unordered_map>

/**
 * @brief Class that manages the rendering of scenes and all the associated 
 * rendering algorithms and data structures.
 */
class CRenderModule : public IModule
{
public:
    CRenderModule();
    CRenderModule(const CRenderModule&) = delete;

    virtual bool Initialize() override;
    virtual void Update() override;
    virtual bool Shutdown() override;

    void HandleWindowResize();
    // TODO: Design a generic way to handle input. Use layers (if an input is consumed in a top layer, do not go to the next layer) and my own
    // enum to represent keys.

    eRenderPath GetRenderPath();
    inline CCamera* GetCamera() const { return m_pMainCamera; };

private:
    void Render();

    void CreateDefaultScene();

    static std::unordered_map<std::string, sRenderObjectInfo> m_RenderObjectInfos;

    CCamera* m_pMainCamera;

    // TODO: An scene is fed to the renderer but should not be part of it. What should be part of it, is a processed scene (for scene graph use purposes for example).
    CScene* m_pDefaultScene;

    // TODO: This backend in the future, could be other graphics API.
    std::unique_ptr<CVulkanBackend> m_pVulkanBackend;

    eRenderPath m_CurrentRenderPath;
};
