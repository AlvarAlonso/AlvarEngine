#pragma once

#include "vk_types.hpp"
#include <string>

class CVulkanDevice;

namespace vkutils
{
    bool LoadShaderModule(VkDevice aDevice, const char* aFilePath, VkShaderModule* aOutShaderModule);

    // TODO: Return AllocatedBuffer instead of passing it in by reference.
    void CreateVertexBuffer(const CVulkanDevice* const aVulkanDevice, const std::vector<sVertex>& aVertices, AllocatedBuffer& aOutBuffer);

    void CreateIndexBuffer(const CVulkanDevice* const aVulkanDevice, const std::vector<uint32_t>& aIndices, AllocatedBuffer& aOutBuffer);

    AllocatedBuffer CreateBuffer(const CVulkanDevice* const aVulkanDevice, size_t aAllocSize, VkBufferUsageFlags aUsage, VmaMemoryUsage aMemoryUsage, VmaAllocationCreateFlags aFlags = 0);

    bool LoadImageFromFile(const CVulkanDevice* const aVulkanDevice, const std::string& aFile, AllocatedImage& aOutImage);

    bool LoadMeshFromFile(const std::string& aFilename, sMesh& aOutMesh);
}