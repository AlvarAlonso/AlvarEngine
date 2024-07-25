#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <renderer/core/render_types.hpp>

#include <vector>

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

struct sMesh
{ 
public:
    static sMesh* GetMesh(const std::string& aID);
    static bool HasMesh(const std::string& aID);

    sMesh(const std::string& aID);
    ~sMesh();

    std::string ID;
    AllocatedBuffer VertexBuffer;
    AllocatedBuffer IndexBuffer;
    uint32_t NumIndices;

private:
    static std::unordered_map<std::string, sMesh*> LoadedMeshes;

};