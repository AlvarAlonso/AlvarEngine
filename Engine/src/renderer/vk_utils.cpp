#include "vk_utils.hpp"

#include "engine.hpp"

#include <iostream>
#include <fstream>

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

void vkutils::CreateVertexBuffer(VmaAllocator aVmaAllocator, const std::vector<sVertex>& aVertices, AllocatedBuffer& aOutBuffer)
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

	VK_CHECK(vmaCreateBuffer(aVmaAllocator, &StagingBufferInfo, &VmaAllocInfo, 
		&StagingBuffer.Buffer, &StagingBuffer.Allocation, nullptr));

	void* Data;
	vmaMapMemory(aVmaAllocator, StagingBuffer.Allocation, &Data);
	memcpy(Data, aVertices.data(), BufferSize);
	vmaUnmapMemory(aVmaAllocator, StagingBuffer.Allocation);

	VkBufferCreateInfo VertexBufferInfo = {};
	VertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	VertexBufferInfo.pNext = nullptr;
	VertexBufferInfo.size = BufferSize;
	VertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

	VmaAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	// TODO: Resource Leak. Destroy this buffer somewhere.
	VK_CHECK(vmaCreateBuffer(aVmaAllocator, &VertexBufferInfo, &VmaAllocInfo,
		&aOutBuffer.Buffer, &aOutBuffer.Allocation, nullptr));

	// TODO: Prefix classes.
	VulkanModule* vulkanModule = Engine::Get()->GetVulkanModule();
	assert(vulkanModule);

	vulkanModule->ImmediateSubmit([=](VkCommandBuffer Cmd)
	{
		VkBufferCopy Copy;
		Copy.dstOffset = 0;
		Copy.srcOffset = 0;
		Copy.size = BufferSize;

		vkCmdCopyBuffer(Cmd, StagingBuffer.Buffer, aOutBuffer.Buffer, 1, &Copy);
	});

	vmaDestroyBuffer(aVmaAllocator, StagingBuffer.Buffer, StagingBuffer.Allocation);
}

void vkutils::CreateIndexBuffer(VmaAllocator aVmaAllocator, const std::vector<uint32_t>& aIndices, AllocatedBuffer& aOutBuffer)
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

	VK_CHECK(vmaCreateBuffer(aVmaAllocator, &StagingBufferInfo, &VmaAllocInfo,
		&StagingBuffer.Buffer, &StagingBuffer.Allocation, nullptr));

	void* Data;
	vmaMapMemory(aVmaAllocator, StagingBuffer.Allocation, &Data);
	memcpy(Data, aIndices.data(), BufferSize);
	vmaUnmapMemory(aVmaAllocator, StagingBuffer.Allocation);

	VkBufferCreateInfo IndexBufferInfo = {};
    IndexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    IndexBufferInfo.pNext = nullptr;
    IndexBufferInfo.size = BufferSize;
    IndexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VmaAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	// TODO: Resource Leak. Destroy this buffer somewhere.
	VK_CHECK(vmaCreateBuffer(aVmaAllocator, &IndexBufferInfo, &VmaAllocInfo,
		&aOutBuffer.Buffer, &aOutBuffer.Allocation, nullptr));

	// TODO: Prefix classes.
	VulkanModule* vulkanModule = Engine::Get()->GetVulkanModule();
	assert(vulkanModule);

	vulkanModule->ImmediateSubmit([=](VkCommandBuffer Cmd)
	{
		VkBufferCopy Copy;
		Copy.dstOffset = 0;
		Copy.srcOffset = 0;
		Copy.size = BufferSize;

		vkCmdCopyBuffer(Cmd, StagingBuffer.Buffer, aOutBuffer.Buffer, 1, &Copy);
	});

	vmaDestroyBuffer(aVmaAllocator, StagingBuffer.Buffer, StagingBuffer.Allocation);
}

AllocatedBuffer vkutils::CreateBuffer(VmaAllocator aVmaAllocator, size_t aAllocSize, VkBufferUsageFlags aUsage, VmaMemoryUsage aMemoryUsage, VmaAllocationCreateFlags aFlags)
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

	VK_CHECK(vmaCreateBuffer(aVmaAllocator, &BufferInfo, &VmaAllocInfo,
		&NewBuffer.Buffer,
		&NewBuffer.Allocation,
		nullptr));

	return NewBuffer;
}
