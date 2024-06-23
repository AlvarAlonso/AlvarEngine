#pragma once

#include "vk_types.hpp"
#include <string>

namespace vkutils
{
    bool LoadShaderModule(VkDevice aDevice, const char* aFilePath, VkShaderModule* aOutShaderModule);

    // TODO: Return AllocatedBuffer instead of passing it in by reference.
    void CreateVertexBuffer(VmaAllocator aVmaAllocator, const std::vector<sVertex>& aVertices, AllocatedBuffer& aOutBuffer);

    void CreateIndexBuffer(VmaAllocator aVmaAllocator, const std::vector<uint32_t>& aIndices, AllocatedBuffer& aOutBuffer);

    AllocatedBuffer CreateBuffer(VmaAllocator aVmaAllocator, size_t aAllocSize, VkBufferUsageFlags aUsage, VmaMemoryUsage aMemoryUsage, VmaAllocationCreateFlags aFlags = 0);

    bool LoadImageFromFile(VmaAllocator aVmaAllocator, const std::string& File, AllocatedImage& aOutImage);
}