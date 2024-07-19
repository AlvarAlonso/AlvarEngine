#pragma once

#include "Vulkan/vulkan_backend.hpp"
#include "scene.hpp"
#include "core/camera.hpp"

// TODO: There should be no windows code in here.
#include <memory>
#include <unordered_map>

/**
 * @brief Class that manages the rendering of scenes and all the associated 
 * rendering algorithms and data structures.
 */
class CRenderModule
{
public:
    CRenderModule();
    CRenderModule(const CRenderModule&) = delete;

    bool Initialize();
    void Render();
    bool Shutdown();
    void HandleWindowResize();
    // TODO: Design a generic way to handle input. Use layers (if an input is consumed in a top layer, do not go to the next layer) and my own
    // enum to represent keys.

private:
    void CreateDefaultScene();

    CCamera* m_pMainCamera;

    CScene* m_pDefaultScene;
    static std::unordered_map<std::string, sRenderObjectInfo> m_RenderObjectInfos;

    // TODO: This backend in the future, could be other graphics API.
    std::unique_ptr<CVulkanBackend> m_pVulkanBackend;
};
