#pragma once

#include "Vulkan/vulkan_backend.hpp"
#include "scene.hpp"

#include <windows.h> 
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

    bool Initialize(const HINSTANCE aInstanceHandle, const HWND aWindowHandle);
    void Render();
    bool Shutdown();
    void HandleWindowResize();

private:
    void CreateDefaultScene();

    CScene* m_pDefaultScene;
    static std::unordered_map<std::string, sRenderObjectInfo> m_RenderObjectInfos;

    // TODO: This backend in the future, could be other graphics API.
    std::unique_ptr<CVulkanBackend> m_pVulkanBackend;
};
