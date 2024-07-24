#pragma once

#include "vk_types.hpp"
#include <renderer/render_pipeline/IRenderPath.hpp>
#include <core/types.hpp>
#include "vulkan_backend.hpp"

class CVulkanDevice;
class CVulkanSwapchain;

struct sGPUCameraData {
	glm::mat4 View;
	glm::mat4 Projection;
	glm::mat4 Viewproj;
};

class CVulkanDeferredRenderPath : public IRenderPath
{
public:
    CVulkanDeferredRenderPath(CVulkanBackend* apVulkanBackend, CVulkanDevice* apVulkanDevice, CVulkanSwapchain* apVulkanSwapchain);
    virtual void CreateResources() override;
    virtual void DestroyResources() override;
    virtual void Render(const CCamera* const aCamera) override;
    virtual void UpdateBuffers() override;
    virtual void HandleSceneChanged() override;

private:
    void CreateDeferredQuad();
    void CreateDeferredAttachments();
    void CreateGBufferDescriptors();
    void CreateDeferredRenderPass();
    void CreateGBufferFramebuffer();
    void CreateDeferredPipeline();
    void CreateDeferredCommandStructures();
    void CreateDeferredSyncrhonizationStructures();

    void RecordLightPassCommands(VkCommandBuffer aCommandBuffer, uint32_t aImageIdx);
    void RecordGBufferPassCommands();

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

    VkFramebuffer m_GBufferFramebuffer;
    VkRenderPass m_DeferredRenderPass;

    VkDescriptorSetLayout m_GBufferSetLayout;
    VkDescriptorSetLayout m_CameraSetLayout;
    VkDescriptorPool m_DeferredDescriptorPool;
    VkDescriptorSet m_GBufferDescriptorSet;
    VkDescriptorSet m_CameraDescriptorSet;
    AllocatedBuffer m_CameraBuffer;

    VkPipelineLayout m_DeferredPipelineLayout;
    VkPipelineLayout m_LightPipelineLayout;
    VkPipeline m_DeferredPipeline;
    VkPipeline m_LightPipeline;

    VkCommandPool m_DeferredCommandPool;
    VkCommandBuffer m_DeferredCommandBuffer;
    VkSemaphore m_GBufferReadySemaphore;

    sRenderObjectData m_Quad;

    // Holds the deletion functions.
    sDeletionQueue m_MainDeletionQueue;
};