#include "vk_utils.hpp"

#include "engine.hpp"
#include <core/logger.h>
#include "vk_initializers.hpp"
#include "vulkan_device.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <iostream>
#include <fstream>
#include <unordered_map>

bool vkutils::LoadShaderModule(VkDevice aDevice, const char* aFilePath, VkShaderModule* aOutShaderModule)
{
	std::ifstream File(aFilePath, std::ios::ate | std::ios::binary);

	if (!File.is_open()) {
		return false;
	}

	size_t FileSize = (size_t)File.tellg();

	std::vector<uint32_t> Buffer(FileSize / sizeof(uint32_t));

	File.seekg(0);

	File.read((char*)Buffer.data(), FileSize);

	File.close();

	VkShaderModuleCreateInfo CreateInfo = {};
	CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	CreateInfo.pNext = nullptr;

	CreateInfo.codeSize = Buffer.size() * sizeof(uint32_t);
	CreateInfo.pCode = Buffer.data();

	VkShaderModule ShaderModule;
	if (vkCreateShaderModule(aDevice, &CreateInfo, nullptr, &ShaderModule) != VK_SUCCESS)
	{
		return false;
	}

	*aOutShaderModule = ShaderModule;
	return true;
}

void vkutils::CreateVertexBuffer(const CVulkanDevice* const aVulkanDevice, const std::vector<sVertex>& aVertices, AllocatedBuffer& aOutBuffer)
{
	const size_t BufferSize = aVertices.size() * sizeof(sVertex);

	VkBufferCreateInfo StagingBufferInfo = {};
	StagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	StagingBufferInfo.pNext = nullptr;
	StagingBufferInfo.size = BufferSize;
	StagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VmaAllocationCreateInfo VmaAllocInfo = {};
	VmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

	AllocatedBuffer StagingBuffer;

	VmaAllocator Allocator = aVulkanDevice->m_Allocator;

	VK_CHECK(vmaCreateBuffer(Allocator, &StagingBufferInfo, &VmaAllocInfo, 
		&StagingBuffer.Buffer, &StagingBuffer.Allocation, nullptr));

	void* Data;
	vmaMapMemory(Allocator, StagingBuffer.Allocation, &Data);
	memcpy(Data, aVertices.data(), BufferSize);
	vmaUnmapMemory(Allocator, StagingBuffer.Allocation);

	VkBufferCreateInfo VertexBufferInfo = {};
	VertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	VertexBufferInfo.pNext = nullptr;
	VertexBufferInfo.size = BufferSize;
	VertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

	VmaAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	// It is responsibility of the caller to delete this.
	VK_CHECK(vmaCreateBuffer(Allocator, &VertexBufferInfo, &VmaAllocInfo,
		&aOutBuffer.Buffer, &aOutBuffer.Allocation, nullptr));

	aVulkanDevice->ImmediateSubmit([=](VkCommandBuffer Cmd)
	{
		VkBufferCopy Copy;
		Copy.dstOffset = 0;
		Copy.srcOffset = 0;
		Copy.size = BufferSize;

		vkCmdCopyBuffer(Cmd, StagingBuffer.Buffer, aOutBuffer.Buffer, 1, &Copy);
	});

	vmaDestroyBuffer(Allocator, StagingBuffer.Buffer, StagingBuffer.Allocation);
}

void vkutils::CreateIndexBuffer(const CVulkanDevice* const aVulkanDevice, const std::vector<uint32_t>& aIndices, AllocatedBuffer& aOutBuffer)
{
	const size_t BufferSize = aIndices.size() * sizeof(uint32_t);

	VkBufferCreateInfo StagingBufferInfo = {};
	StagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	StagingBufferInfo.pNext = nullptr;
	StagingBufferInfo.size = BufferSize;
	StagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VmaAllocationCreateInfo VmaAllocInfo = {};
	VmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

	AllocatedBuffer StagingBuffer;

	VmaAllocator Allocator = aVulkanDevice->m_Allocator;

	VK_CHECK(vmaCreateBuffer(Allocator, &StagingBufferInfo, &VmaAllocInfo,
		&StagingBuffer.Buffer, &StagingBuffer.Allocation, nullptr));

	void* Data;
	vmaMapMemory(Allocator, StagingBuffer.Allocation, &Data);
	memcpy(Data, aIndices.data(), BufferSize);
	vmaUnmapMemory(Allocator, StagingBuffer.Allocation);

	VkBufferCreateInfo IndexBufferInfo = {};
    IndexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    IndexBufferInfo.pNext = nullptr;
    IndexBufferInfo.size = BufferSize;
    IndexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VmaAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	// It is responsibility of the caller to delete this.
	VK_CHECK(vmaCreateBuffer(Allocator, &IndexBufferInfo, &VmaAllocInfo,
		&aOutBuffer.Buffer, &aOutBuffer.Allocation, nullptr));

	aVulkanDevice->ImmediateSubmit([=](VkCommandBuffer Cmd)
	{
		VkBufferCopy Copy;
		Copy.dstOffset = 0;
		Copy.srcOffset = 0;
		Copy.size = BufferSize;

		vkCmdCopyBuffer(Cmd, StagingBuffer.Buffer, aOutBuffer.Buffer, 1, &Copy);
	});

	vmaDestroyBuffer(Allocator, StagingBuffer.Buffer, StagingBuffer.Allocation);
}

AllocatedBuffer vkutils::CreateBuffer(const CVulkanDevice* const aVulkanDevice, size_t aAllocSize, VkBufferUsageFlags aUsage, VmaMemoryUsage aMemoryUsage, VmaAllocationCreateFlags aFlags)
{
	VkBufferCreateInfo BufferInfo = {};
	BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferInfo.pNext = nullptr;

	BufferInfo.size = aAllocSize;
	BufferInfo.usage = aUsage;

	VmaAllocationCreateInfo VmaAllocInfo = {};
	VmaAllocInfo.usage = aMemoryUsage;
	VmaAllocInfo.flags = aFlags;

	AllocatedBuffer NewBuffer;

	VK_CHECK(vmaCreateBuffer(aVulkanDevice->m_Allocator, &BufferInfo, &VmaAllocInfo,
		&NewBuffer.Buffer,
		&NewBuffer.Allocation,
		nullptr));

	return NewBuffer;
}

bool vkutils::LoadImageFromFile(const CVulkanDevice* const aVulkanDevice, const std::string& File, AllocatedImage& aOutImage)
{
	int32_t TexWidth, TexHeight, TexChannels;

	stbi_uc* Pixels = stbi_load(File.c_str(), &TexWidth, &TexHeight, &TexChannels, STBI_rgb_alpha);

	if (!Pixels)
	{
		SGSERROR("Failed to load texture file: %s.", File.c_str());
		return false;
	}

	void* Pixel_Ptr = Pixels;
	const uint64_t ImageSize = TexWidth * TexHeight * 4;

    UploadImageToVRAM(aVulkanDevice, ImageSize, Pixel_Ptr, TexWidth, TexHeight, aOutImage);
    
	stbi_image_free(Pixels);

    return true;
}

void vkutils::UploadImageToVRAM(const CVulkanDevice *const aVulkanDevice, const uint64_t aImageSize, void *aPixel_Ptr, int32_t aTexWidth, int32_t aTexHeight, AllocatedImage &aOutImage)
{
    // TODO: Should all images have this format?
    VkFormat ImageFormat = VK_FORMAT_R8G8B8A8_SRGB;

    VmaAllocator Allocator = aVulkanDevice->m_Allocator;

    AllocatedBuffer StagingBuffer = vkutils::CreateBuffer(aVulkanDevice, aImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void *Data;
    vmaMapMemory(Allocator, StagingBuffer.Allocation, &Data);
    memcpy(Data, aPixel_Ptr, static_cast<size_t>(aImageSize));
    vmaUnmapMemory(Allocator, StagingBuffer.Allocation);

    VkExtent3D ImageExtent;
    ImageExtent.width = static_cast<uint32_t>(aTexWidth);
    ImageExtent.height = static_cast<uint32_t>(aTexHeight);
    ImageExtent.depth = 1;

    VkImageCreateInfo ImageInfo = vkinit::ImageCreateInfo(ImageFormat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, ImageExtent);

    AllocatedImage NewImage;

    VmaAllocationCreateInfo ImageAllocInfo = {};
    ImageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    // It is responsibility of the caller to destroy the image.
    vmaCreateImage(Allocator, &ImageInfo, &ImageAllocInfo, &NewImage.Image, &NewImage.Allocation, nullptr);

    aVulkanDevice->ImmediateSubmit([&](VkCommandBuffer aCmd)
                                   {
		VkImageSubresourceRange Range;
		Range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		Range.baseMipLevel = 0;
		Range.levelCount = 1;
		Range.baseArrayLayer = 0;
		Range.layerCount = 1;

		VkImageMemoryBarrier ImageBarrierToTransfer = {};
		ImageBarrierToTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		ImageBarrierToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ImageBarrierToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		ImageBarrierToTransfer.image = NewImage.Image;
		ImageBarrierToTransfer.subresourceRange = Range;
		ImageBarrierToTransfer.srcAccessMask = 0;
		ImageBarrierToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(aCmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &ImageBarrierToTransfer);

		VkBufferImageCopy CopyRegion = {};
		CopyRegion.bufferOffset = 0;
		CopyRegion.bufferRowLength = 0;
		CopyRegion.bufferImageHeight = 0;
		CopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		CopyRegion.imageSubresource.mipLevel = 0;
		CopyRegion.imageSubresource.baseArrayLayer = 0;
		CopyRegion.imageSubresource.layerCount = 1;
		CopyRegion.imageExtent = ImageExtent;

		vkCmdCopyBufferToImage(aCmd, StagingBuffer.Buffer, NewImage.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &CopyRegion);
	
		VkImageMemoryBarrier ImageBarrierToReadable = ImageBarrierToTransfer;
		ImageBarrierToReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		ImageBarrierToReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		ImageBarrierToReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		ImageBarrierToReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(aCmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &ImageBarrierToReadable); });

    vmaDestroyBuffer(Allocator, StagingBuffer.Buffer, StagingBuffer.Allocation);

    aOutImage = NewImage;
}

size_t vkutils::GetAlignedSize(size_t aOriginalSize, size_t aAlignment)
{
	size_t AlignedSize = aOriginalSize;
	if (aAlignment > 0)
	{
		AlignedSize = (AlignedSize + aAlignment - 1) & ~(aAlignment - 1);
	}

	return AlignedSize;
}
