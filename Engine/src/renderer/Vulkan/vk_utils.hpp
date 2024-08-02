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

    bool LoadImageFromFile(const CVulkanDevice *const aVulkanDevice, const std::string &aFile, AllocatedImage &aOutImage);

    void UploadImageToVRAM(const CVulkanDevice *const aVulkanDevice, const uint64_t aImageSize, void *aPixel_Ptr, int32_t aTexWidth, int32_t aTexHeight, AllocatedImage &aOutImage);

    size_t GetAlignedSize(size_t aOriginalSize, size_t aAlignment);
}