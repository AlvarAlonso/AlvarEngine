#pragma once

#include "vk_types.hpp"

namespace vkutils
{
    bool LoadShaderModule(VkDevice aDevice, const char* aFilePath, VkShaderModule* aOutShaderModule);

    void CreateVertexBuffer(VmaAllocator aVmaAllocator, const std::vector<sVertex>& aVertices, AllocatedBuffer& aOutBuffer);
}