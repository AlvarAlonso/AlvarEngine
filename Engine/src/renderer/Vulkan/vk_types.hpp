#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <string>
#include <vector>
#include <unordered_map>

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
    static std::unordered_map<std::string, sMesh*> LoadedMeshes;
    static sMesh* GetMesh(const std::string& aFilename);

    // TODO: Vulkan should only know about buffers. Once the buffers are loaded, the Vertices and Indices vectors are useless.
    // Maybe this variables should be deleted and have a vulkan representation of the mesh and a resource representation of the mesh.
    std::vector<sVertex> Vertices;
    std::vector<uint32_t> Indices;

    AllocatedBuffer VertexBuffer;
    AllocatedBuffer IndexBuffer;
};