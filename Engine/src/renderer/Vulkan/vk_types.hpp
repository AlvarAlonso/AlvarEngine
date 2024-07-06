#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

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

struct sVertex
{
    glm::vec3 Position;
    glm::vec3 Color;
    glm::vec2 UV;

    static sVertexInputDescription GetVertexDescription();

    bool operator==(const sVertex& aOther) const 
    {
        return Position == aOther.Position && UV == aOther.UV;
    }
};

namespace std {
	template<> struct hash<sVertex> {
		size_t operator()(sVertex const& Vertex) const {
			return ((hash<glm::vec3>()(Vertex.Position) ^
				(hash<glm::vec3>()(Vertex.Color) << 1)) >> 1) ^
				(hash<glm::vec2>()(Vertex.UV) << 1);
		}
	};
}

struct sMesh
{
    std::vector<sVertex> Vertices;
    std::vector<uint32_t> Indices;

    AllocatedBuffer VertexBuffer;
    AllocatedBuffer IndexBuffer;
};