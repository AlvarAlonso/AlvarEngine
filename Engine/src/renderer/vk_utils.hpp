#pragma once

#include "vk_types.hpp"

namespace vkutils
{
    bool LoadShaderModule(VkDevice aDevice, const char* aFilePath, VkShaderModule* aOutShaderModule);

    void CreateVertexBuffer(VmaAllocator aVmaAllocator, const std::vector<sVertex>& aVertices, AllocatedBuffer& aOutBuffer);

    void CreateIndexBuffer(VmaAllocator aVmaAllocator, const std::vector<uint32_t>& aIndices, AllocatedBuffer& aOutBuffer);
}