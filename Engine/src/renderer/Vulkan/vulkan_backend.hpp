#pragma once

#include "vk_types.hpp"
#include "renderer/scene.hpp"

class CVulkanDevice;
class CVulkanSwapchain;
class CCamera;

constexpr uint32_t MAX_RENDER_OBJECTS = 1024;
constexpr uint32_t FRAME_OVERLAP = 3;

struct sFrameData
{
    VkSemaphore PresentSemaphore;
    VkSemaphore RenderSemaphore;
    VkFence RenderFence;
    VkCommandPool CommandPool;
    VkCommandBuffer MainCommandBuffer;
    AllocatedBuffer UBOBuffer;
    void* MappedUBOBuffer;
    VkDescriptorSet DescriptorSet;
};

struct sRenderObjectData
{
    glm::mat4 ModelMatrix;
    sMesh* pMesh;
};

struct sGPURenderObjectData
{
    glm::mat4 ModelMatrix;
};

struct sFrameUBO
{
    glm::mat4 View;
    glm::mat4 Proj;
    glm::mat4 ViewProj;
};

/**
 * @brief Class that manages all Vulkan initialization and resource management. It serves the RenderModule
 * with all the resources it needs to perform its algorithms, and call the actual commands that will be sent to GPU.
 */
class CVulkanBackend
{
public:
    CVulkanBackend();
    
    bool Initialize(const HINSTANCE aInstanceHandle, const HWND aWindowHandle);

    void Render(const CCamera* const aCamera);

    bool Shutdown();

    void HandleWindowResize();

    void CreateRenderObjectsData(const std::vector<sRenderObject*>& aRenderObjects);

private:
    void InitCommandPools();
    void InitSyncStructures();
    void InitTextureSamplers();
    void InitDescriptorSetLayouts();
    void InitDescriptorSetPool();
    void InitDescriptorSets();
    void InitPipelines();

    void UpdateFrameUBO(const CCamera* const aCamera, uint32_t ImageIdx);
    void RecordCommandBuffer(VkCommandBuffer aCommandBuffer, uint32_t aImageIdx);
    void RenderFrame(const CCamera* const aCamera);
    
    bool HasStencilComponent(VkFormat aFormat);

    bool m_bIsInitialized;

    CVulkanDevice* m_VulkanDevice;
    CVulkanSwapchain* m_VulkanSwapchain;

    VkCommandPool m_CommandPool;
    // ------------

    // Descriptors.
    VkDescriptorPool m_DescriptorPool;
    VkDescriptorSetLayout m_DescriptorSetLayout;
    VkDescriptorSetLayout m_RenderObjectsSetLayout;
    // ------------
    
    // Pipelines.
    VkPipelineLayout m_ForwardPipelineLayout;
    VkPipeline m_ForwardPipeline;
    //-------------

    sFrameData m_FramesData[FRAME_OVERLAP];
    uint32_t m_CurrentFrame;

    bool m_bWasWindowResized;

    // TODO: Some way to represent a Scene.
    std::vector<sRenderObjectData> m_RenderObjectsData;
    AllocatedBuffer m_ObjectsDataBuffer;
    VkDescriptorSet m_ObjectsDataDescriptorSet;

    VkImageView m_ImageView;
    VkSampler m_DefaultSampler;
    AllocatedImage m_Image;
};