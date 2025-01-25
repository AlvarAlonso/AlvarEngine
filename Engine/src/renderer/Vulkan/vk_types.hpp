#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <renderer/core/render_types.hpp>
#include <renderer/resources/material.hpp>
#include <renderer/scene/entities/light_source.hpp> // TODO: Vulkan no hauria d'estar incloent coses específiques de renderer.
#include <vector>

#include <iostream>

class CVkTexture;

#define VK_CHECK(x)                                                 \
    do                                                              \
    {                                                               \
        VkResult err = x;                                           \
        if (err)                                                    \
        {                                                           \
            std::cout <<"Detected Vulkan error: " << err << std::endl; \
            std::abort();                                                \
        }                                                           \
    } while (0)														\
    
struct AllocatedBuffer
{
	VkBuffer Buffer;
    VmaAllocation Allocation;
};

struct AllocatedImage 
{
    VkImage Image;
    VmaAllocation Allocation;
};

struct sVertexInputDescription
{
    std::vector<VkVertexInputBindingDescription> Bindings;
    std::vector<VkVertexInputAttributeDescription> Attributes;
};

sVertexInputDescription GetVertexDescription();

struct sMaterialResources
{
    CVkTexture* pAlbedoTexture;
    CVkTexture* pMetalRoughnessTexture;
    CVkTexture* pEmissiveTexture;
    CVkTexture* pNormalTexture;
};

struct sMaterialDescriptor
{
    CMaterial* pMaterial;
    sMaterialResources Resources;
    AllocatedBuffer ConstantsBuffer;
    VkDescriptorSet DescriptorSet;
};

struct sRenderContext
{
    VkPipelineLayout PipelineLayout;
    VkCommandBuffer CmdBuffer;
    VkDescriptorSet FrameDescriptorSet;
    VkDescriptorSet ObjectsDescriptorSet;
    std::unordered_map<std::string, sMaterialDescriptor*>* MaterialDescriptors; // TODO: This should be get directly from CVulkanBackend.
    VkDescriptorSet LightSourcesDescriptorSet;
    size_t NumLights; // TODO: Find a way to link this to the LightSourcesDescriptorSet so they can't go out of sync.
    uint32_t DrawCallNum;
};

struct sLightData
{
    glm::vec3 Color;
    int Padding;
};

class CVulkanRenderable : public CRenderable
{
public:
    CVulkanRenderable() = default;
    CVulkanRenderable(sMeshData* apMeshData);

    void Draw(sRenderContext& aRenderContext, bool bBindMaterialDescriptor = false);
    virtual void UploadToVRAM() override;

    AllocatedBuffer m_VertexBuffer;
    AllocatedBuffer m_IndexBuffer;

private:
    void DrawNode(CMeshNode* apMeshNode, sRenderContext& aRenderContext, bool bBindMaterialDescriptor = false);
    void DrawSubMesh(CSubMesh* apSubMesh, sRenderContext& aRenderContext, bool bBindMaterialDescriptor = false);
};
