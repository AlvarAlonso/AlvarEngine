#pragma once

#include "vk_types.hpp"
#include <renderer/render_pipeline/IRenderPath.hpp>
#include <core/types.hpp>

class CVulkanBackend;
class CVulkanDevice;
class CVulkanSwapchain;

class CVulkanForwardRenderPath : public IRenderPath
{
public:
    CVulkanForwardRenderPath(CVulkanBackend* apVulkanBackend, CVulkanDevice* apVulkanDevice, CVulkanSwapchain* apVulkanSwapchain);
    virtual void CreateResources() override;
    virtual void DestroyResources() override;
    virtual void Render(const CCamera* const aCamera) override;
    virtual void UpdateBuffers() override {};
    virtual void HandleSceneChanged() override;

    VkPipeline m_ForwardPipeline;
    VkPipelineLayout m_ForwardPipelineLayout;
    
private:
    void CreateForwardPipeline();
    void RecordCommands(VkCommandBuffer aCommandBuffer, uint32_t aImageIdx);

    CVulkanBackend* m_pVulkanBackend;
    CVulkanDevice* m_pVulkanDevice;
    CVulkanSwapchain* m_pVulkanSwapchain;

    // Holds the deletion functions.
    sDeletionQueue m_MainDeletionQueue;
};