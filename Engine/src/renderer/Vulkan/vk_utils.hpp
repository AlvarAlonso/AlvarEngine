#pragma once

#include "vk_types.hpp"
#include <string>

class CVulkanBackend;

namespace vkutils
{
    bool LoadShaderModule(VkDevice aDevice, const char* aFilePath, VkShaderModule* aOutShaderModule);

    // TODO: Return AllocatedBuffer instead of passing it in by reference.
    void CreateVertexBuffer(const CVulkanBackend* const aVulkanBackend, VmaAllocator aVmaAllocator, const std::vector<sVertex>& aVertices, AllocatedBuffer& aOutBuffer);

    void CreateIndexBuffer(const CVulkanBackend* const aVulkanBackend, VmaAllocator aVmaAllocator, const std::vector<uint32_t>& aIndices, AllocatedBuffer& aOutBuffer);

    AllocatedBuffer CreateBuffer(VmaAllocator aVmaAllocator, size_t aAllocSize, VkBufferUsageFlags aUsage, VmaMemoryUsage aMemoryUsage, VmaAllocationCreateFlags aFlags = 0);

    bool LoadImageFromFile(const CVulkanBackend* const aVulkanBackend, VmaAllocator aVmaAllocator, const std::string& aFile, AllocatedImage& aOutImage);

    bool LoadMeshFromFile(const std::string& aFilename, sMesh& aOutMesh);
}