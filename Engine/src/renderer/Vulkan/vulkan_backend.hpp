#pragma once

#include "vk_types.hpp"
#include "renderer/scene.hpp"

class CVulkanDevice;
class CVulkanSwapchain;
class CCamera;
class IRenderPath;

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
    
    bool Initialize();

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

    IRenderPath* CreateRenderPath();
    void InitRenderPath(IRenderPath* aRenderPath);

    void UpdateFrameUBO(const CCamera* const aCamera, uint32_t ImageIdx);
    
    bool HasStencilComponent(VkFormat aFormat);

    friend class CVulkanDeferredRenderPath;
    friend class CVulkanForwardRenderPath;

    bool m_bIsInitialized;

    CVulkanDevice* m_pVulkanDevice;
    CVulkanSwapchain* m_pVulkanSwapchain;

    IRenderPath* m_pCurrentRenderPath;

    VkDescriptorSetLayout m_DescriptorSetLayout;
    VkDescriptorSetLayout m_RenderObjectsSetLayout;
    VkSampler m_DefaultSampler;

    VkCommandPool m_CommandPool;

    VkDescriptorPool m_DescriptorPool;

    sFrameData m_FramesData[FRAME_OVERLAP];
    uint32_t m_CurrentFrame;

    bool m_bWasWindowResized;

    // TODO: Some way to represent a Scene.
    std::vector<sRenderObjectData> m_RenderObjectsData;
    AllocatedBuffer m_ObjectsDataBuffer;
    VkDescriptorSet m_ObjectsDataDescriptorSet;

    VkImageView m_ImageView;
    AllocatedImage m_Image;
};