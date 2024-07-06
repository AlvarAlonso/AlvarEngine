#pragma once

#include "Vulkan/vulkan_backend.hpp"

#include <windows.h> 
#include <memory>

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
    // TODO: This backend in the future, could be other graphics API.
    std::unique_ptr<CVulkanBackend> m_VulkanBackend;
};