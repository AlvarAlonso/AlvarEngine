#pragma once

#include "vk_types.hpp"
#include <renderer/scene/scene.hpp>
#include <core/types.hpp>
#include <memory>

namespace Alvar
{
    class CVulkanDevice;
    class CVulkanSwapchain;
    class CCamera;
    class IRenderPath;
    class CRenderable;
    struct sLightSource;

    constexpr uint32_t MAX_RENDER_OBJECTS = 1024;
    constexpr uint32_t MAX_LIGHT_SOURCES = 24;
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

        void Render(const std::weak_ptr<CCamera> apCamera);

        bool Shutdown();

        void HandleWindowResize();

        void CreateRenderablesData(const CScene* const apScene);

        void ChangeRenderPath();

        CVulkanDevice *GetDevice() const { return m_pVulkanDevice; }

        // TODO: Abstract this.
        void InitImGuiBackend();
        void ShutdownImGuiBackend();
        void ImGuiBeginFrame();
        void ImGuiEndFrame();

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
        void UpdateFrameUBO(const std::weak_ptr<CCamera> apCamera, uint32_t ImageIdx);
        
        bool HasStencilComponent(VkFormat aFormat);

        void AddTransformsToBuffer(sGPURenderObjectData* apBuffer, size_t& aIndex, CMeshNode* apMeshNode);
    
        void CreateMaterialDescriptorsFromMeshNodeRecursive(CMeshNode *const &aMeshNode);
        void CreateMaterialDescriptorsFromMeshNode(CMeshNode *const &aMeshNode);

        void OnBeforeRenderEnd(VkCommandBuffer aCommandBuffer);

        uint32_t GetImGuiVulkanPoolSize();

        friend class CVulkanDeferredRenderPath;
        friend class CVulkanForwardRenderPath;

        bool m_bIsInitialized;

        CVulkanDevice* m_pVulkanDevice;
        CVulkanSwapchain* m_pVulkanSwapchain;

        IRenderPath* m_pCurrentRenderPath;

        // TODO: Each render path should have its own memory layout.
        VkDescriptorSetLayout m_DescriptorSetLayout;
        VkDescriptorSetLayout m_RenderObjectsSetLayout;
        VkDescriptorSetLayout m_MaterialsSetLayout;
        VkDescriptorSetLayout m_LightSourceSetLayout;
        VkSampler m_DefaultSampler;

        VkCommandPool m_CommandPool;

        VkDescriptorPool m_DescriptorPool;
        VkDescriptorPool m_MaterialsPool;
        VkDescriptorPool m_LightsPool;

        sFrameData m_FramesData[FRAME_OVERLAP];
        uint32_t m_CurrentFrame;

        bool m_bWasWindowResized;

        // TODO: Some way to represent a Scene.
        std::vector<CVulkanRenderable*> m_Renderables;
        AllocatedBuffer m_ObjectsDataBuffer;
        VkDescriptorSet m_ObjectsDataDescriptorSet;
        AllocatedBuffer m_LightSourcesBuffer;
        VkDescriptorSet m_LightSourcesDescriptorSet;
        size_t m_NumLightSources;

        std::unordered_map<std::string, sMaterialDescriptor*> m_MaterialDescriptors;
        // ------------------------------------

        // TODO: To be removed.
        VkImageView m_ImageView;
        AllocatedImage m_Image;

        // Holds the deletion functions.
        sDeletionQueue m_MainDeletionQueue;
    };
}
