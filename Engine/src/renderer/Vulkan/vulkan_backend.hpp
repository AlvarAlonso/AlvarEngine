#pragma once

#include "vk_types.hpp"
#include "renderer/scene.hpp"
#include <core/types.hpp>

class CVulkanDevice;
class CVulkanSwapchain;
class CCamera;
class IRenderPath;
class CRenderable;

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

struct sGPURenderObjectData
{
    glm::mat4 ModelMatrix;
};

struct sCameraFrameUBO
{
    glm::mat4 View;
    glm::mat4 Proj;
    glm::mat4 ViewProj;
    glm::vec3 Pos;
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

    void CreateRenderablesData(const std::vector<CRenderable *> &aRenderables);

    void ChangeRenderPath();

    CVulkanDevice *GetDevice() const { return m_pVulkanDevice; }

private:
    void InitCommandPools();
    void InitSyncStructures();
    void InitTextureSamplers();
    void InitDescriptorSetLayouts();
    void InitDescriptorSetPool();
    void InitDescriptorSets();

    IRenderPath* CreateRenderPath();
    void InitRenderPath(IRenderPath* aRenderPath);

    void CreateSceneDescriptorSets();
    void UpdateFrameUBO(const CCamera* const aCamera, uint32_t ImageIdx);
    
    bool HasStencilComponent(VkFormat aFormat);

    void AddTransformsToBuffer(sGPURenderObjectData* apBuffer, size_t& aIndex, CMeshNode* apMeshNode);
   
    void CreateMaterialDescriptorsFromMeshNodeRecursive(CMeshNode *const &aMeshNode);
    void CreateMaterialDescriptorsFromMeshNode(CMeshNode *const &aMeshNode);

    friend class CVulkanDeferredRenderPath;
    friend class CVulkanForwardRenderPath;

    bool m_bIsInitialized;

    CVulkanDevice* m_pVulkanDevice;
    CVulkanSwapchain* m_pVulkanSwapchain;

    IRenderPath* m_pCurrentRenderPath;

    VkDescriptorSetLayout m_DescriptorSetLayout;
    VkDescriptorSetLayout m_RenderObjectsSetLayout;
    VkDescriptorSetLayout m_MaterialsSetLayout;
    VkSampler m_DefaultSampler;

    VkCommandPool m_CommandPool;

    VkDescriptorPool m_DescriptorPool;
    VkDescriptorPool m_MaterialsPool;

    sFrameData m_FramesData[FRAME_OVERLAP];
    uint32_t m_CurrentFrame;

    bool m_bWasWindowResized;

    // TODO: Some way to represent a Scene.
    std::vector<CVulkanRenderable*> m_Renderables;
    AllocatedBuffer m_ObjectsDataBuffer;
    VkDescriptorSet m_ObjectsDataDescriptorSet;

    std::unordered_map<std::string, sMaterialDescriptor*> m_MaterialDescriptors;
    // ------------------------------------

    // TODO: To be removed.
    VkImageView m_ImageView;
    AllocatedImage m_Image;

    // Holds the deletion functions.
    sDeletionQueue m_MainDeletionQueue;
};