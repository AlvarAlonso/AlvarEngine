#include "render_module.hpp"

#include <stdexcept>

CRenderModule::CRenderModule() :
    m_VulkanBackend(nullptr)
{

}

bool CRenderModule::Initialize(const HINSTANCE aInstanceHandle, const HWND aWindowHandle)
{
    m_VulkanBackend = std::make_unique<CVulkanBackend>();
    if (m_VulkanBackend == nullptr)
    {
        throw std::runtime_error("Failed to create the Vulkan Backend!");
    }

    m_VulkanBackend->Initialize(aInstanceHandle, aWindowHandle);

    return true;
}

void CRenderModule::Render()
{
    // TODO: Should this pointer be checked?
    m_VulkanBackend->Render();
}

bool CRenderModule::Shutdown()
{
    return m_VulkanBackend->Shutdown();
}

void CRenderModule::HandleWindowResize()
{
    m_VulkanBackend->HandleWindowResize();
}