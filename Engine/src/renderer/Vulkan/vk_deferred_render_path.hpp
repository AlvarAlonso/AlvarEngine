#pragma once

#include "vk_types.hpp"
#include <renderer/render_pipeline/IRenderPath.hpp>
#include <core/types.hpp>

class CVulkanBackend;
class CVulkanDevice;
class CVulkanSwapchain;

class CVulkanDeferredRenderPath : public IRenderPath
{
public:
    CVulkanDeferredRenderPath(CVulkanBackend* apVulkanBackend, CVulkanDevice* apVulkanDevice, CVulkanSwapchain* apVulkanSwapchain);
    virtual void CreateResources() override;
    virtual void DestroyResources() override;
    virtual void Render(const CCamera* const aCamera) override;

private:
    void CreateDeferredAttachments();
    void CreateGBufferDescriptors();
    void CreateDeferredRenderPass();
    void CreateDeferredPipeline();
    void RecordCommands(VkCommandBuffer aCommandBuffer, uint32_t aImageIdx);

    CVulkanBackend* m_pVulkanBackend;
    CVulkanDevice* m_pVulkanDevice;
    CVulkanSwapchain* m_pVulkanSwapchain;

    VkFormat m_PositionFormat;
    VkFormat m_NormalFormat;
    VkFormat m_AlbedoFormat;

    AllocatedImage m_PositionImage;
    AllocatedImage m_NormalImage;
    AllocatedImage m_AlbedoImage;

    VkImageView m_PositionImageView;
    VkImageView m_NormalImageView;
    VkImageView m_AlbedoImageView;

    VkRenderPass m_DeferredRenderPass;

    VkDescriptorSetLayout m_SingleTextureSetLayout;
    VkDescriptorSetLayout m_GBufferSetLayout;
    VkDescriptorPool m_GBufferPool;
    VkDescriptorSet m_GBufferDescriptorSet;

    VkPipelineLayout m_DeferredPipelineLayout;
    VkPipelineLayout m_LightPipelineLayout;
    VkPipeline m_DeferredPipeline;
    VkPipeline m_LightPipeline;

    // Holds the deletion functions.
    sDeletionQueue m_MainDeletionQueue;
};