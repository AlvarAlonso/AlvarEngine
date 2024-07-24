#include "vk_deferred_render_path.hpp"
#include "vulkan_device.hpp"
#include "vulkan_swapchain.hpp"
#include "vk_initializers.hpp"
#include "vk_utils.hpp"
#include "engine.hpp"
#include <renderer/core/geometry_generator.hpp>
#include <core/logger.h>

#include <iostream>
#include <array>
    
CVulkanDeferredRenderPath::CVulkanDeferredRenderPath(CVulkanBackend* apVulkanBackend, CVulkanDevice* apVulkanDevice, CVulkanSwapchain* apVulkanSwapchain) :
    m_pVulkanBackend(apVulkanBackend), m_pVulkanDevice(apVulkanDevice), m_pVulkanSwapchain(apVulkanSwapchain)
{
}

void CVulkanDeferredRenderPath::CreateResources()
{
	CreateDeferredQuad();
    CreateDeferredAttachments();
    CreateGBufferDescriptors();
    CreateDeferredRenderPass();
	CreateGBufferFramebuffer();
    CreateDeferredPipeline();
	CreateDeferredCommandStructures();
	CreateDeferredSyncrhonizationStructures();
}
    
void CVulkanDeferredRenderPath::DestroyResources()
{
    m_MainDeletionQueue.Flush();
}

void CVulkanDeferredRenderPath::RecordGBufferPassCommands()
{
	VkCommandBufferBeginInfo deferredCmdBeginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

	VK_CHECK(vkBeginCommandBuffer(m_DeferredCommandBuffer, &deferredCmdBeginInfo));

	VkClearValue first_clearValue;
	first_clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

	VkClearValue first_depthClear;
	first_depthClear.depthStencil.depth = 1.0f;

	VkRenderPassBeginInfo rpInfo = vkinit::RenderPassBeginInfo(m_DeferredRenderPass, m_pVulkanSwapchain->m_WindowExtent, m_GBufferFramebuffer);

	std::array<VkClearValue, 4> first_clearValues = { first_clearValue, first_clearValue, first_clearValue, first_depthClear };

	rpInfo.clearValueCount = static_cast<uint32_t>(first_clearValues.size());
	rpInfo.pClearValues = first_clearValues.data();

	vkCmdBeginRenderPass(m_DeferredCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(m_DeferredCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DeferredPipeline);
	
	VkViewport Viewport{};
	Viewport.x = 0.0f;
	Viewport.y = 0.0f;
	Viewport.width = static_cast<float>(m_pVulkanSwapchain->m_WindowExtent.width);
	Viewport.height = static_cast<float>(m_pVulkanSwapchain->m_WindowExtent.height);
	Viewport.minDepth = 0.0f;
	Viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_DeferredCommandBuffer, 0, 1, &Viewport);

	VkRect2D Scissor{};
	Scissor.offset = {0, 0};
	Scissor.extent = m_pVulkanSwapchain->m_WindowExtent;
	vkCmdSetScissor(m_DeferredCommandBuffer, 0, 1, &Scissor);

	std::array<VkDescriptorSet, 2> DescriptorSets = { m_CameraDescriptorSet, m_pVulkanBackend->m_ObjectsDataDescriptorSet };

	vkCmdBindDescriptorSets(m_DeferredCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DeferredPipelineLayout, 0, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data(), 0, nullptr);

	VkDeviceSize Offset = 0;
	for (size_t i = 0; i < m_pVulkanBackend->m_RenderObjectsData.size(); ++i)
	{
		vkCmdBindVertexBuffers(m_DeferredCommandBuffer, 0, 1, &m_pVulkanBackend->m_RenderObjectsData[i].pMesh->VertexBuffer.Buffer, &Offset);
		vkCmdBindIndexBuffer(m_DeferredCommandBuffer, m_pVulkanBackend->m_RenderObjectsData[i].pMesh->IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);
		// TODO: Batch rendering.
		vkCmdDrawIndexed(m_DeferredCommandBuffer, m_pVulkanBackend->m_RenderObjectsData[i].pMesh->NumIndices, 1, 0, 0, i);
	}

	vkCmdEndRenderPass(m_DeferredCommandBuffer);

	VK_CHECK(vkEndCommandBuffer(m_DeferredCommandBuffer));
}

void CVulkanDeferredRenderPath::RecordLightPassCommands(VkCommandBuffer aCommandBuffer, uint32_t aImageIdx)
{
	VK_CHECK(vkResetCommandBuffer(aCommandBuffer, 0));

	VkCommandBufferBeginInfo CmdBeginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(aCommandBuffer, &CmdBeginInfo));
	
	VkRenderPassBeginInfo LightPassBeginInfo = vkinit::RenderPassBeginInfo(m_pVulkanSwapchain->m_RenderPass, m_pVulkanSwapchain->m_WindowExtent, m_pVulkanSwapchain->m_Framebuffers[aImageIdx]);

	std::array<VkClearValue, 2> ClearValues = {};
	ClearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
	ClearValues[1].depthStencil = {1.0f, 0};
	
	LightPassBeginInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
	LightPassBeginInfo.pClearValues = ClearValues.data();

	vkCmdBeginRenderPass(aCommandBuffer, &LightPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_LightPipeline);

	VkViewport Viewport{};
	Viewport.x = 0.0f;
	Viewport.y = 0.0f;
	Viewport.width = static_cast<float>(m_pVulkanSwapchain->m_WindowExtent.width);
	Viewport.height = static_cast<float>(m_pVulkanSwapchain->m_WindowExtent.height);
	Viewport.minDepth = 0.0f;
	Viewport.maxDepth = 1.0f;
	vkCmdSetViewport(aCommandBuffer, 0, 1, &Viewport);

	VkRect2D Scissor{};
	Scissor.offset = {0, 0};
	Scissor.extent = m_pVulkanSwapchain->m_WindowExtent;
	vkCmdSetScissor(aCommandBuffer, 0, 1, &Scissor);

	// TODO: que ous fa aquesta linia.
	//uint32_t uniform_offset = vkutils::GetAlignedSize(sizeof(GPUSceneData) * frameIndex, VulkanEngine::cinstance->_gpuProperties.limits.minUniformBufferOffsetAlignment);
	const std::array<VkDescriptorSet, 2> DescriptorSets = { m_pVulkanBackend->m_FramesData[aImageIdx].DescriptorSet, m_GBufferDescriptorSet };
	vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_LightPipelineLayout, 
		0, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data(), 0, nullptr);

	vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_LightPipelineLayout, 1, 1, &m_GBufferDescriptorSet, 0, nullptr);

	//deferred quad
	VkDeviceSize Offset = 0;
	vkCmdBindVertexBuffers(aCommandBuffer, 0, 1, &m_Quad.pMesh->VertexBuffer.Buffer, &Offset);
	vkCmdBindIndexBuffer(aCommandBuffer, m_Quad.pMesh->IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(aCommandBuffer, m_Quad.pMesh->NumIndices, 1, 0, 0, 0);

	vkCmdEndRenderPass(aCommandBuffer);

	VK_CHECK(vkEndCommandBuffer(aCommandBuffer));
}

void CVulkanDeferredRenderPath::Render(const CCamera* const aCamera)
{
    // TODO: Probably there is a chunk of this code that can go to CVulkanBackend.
	
    VK_CHECK(vkWaitForFences(m_pVulkanDevice->m_Device, 1, &m_pVulkanBackend->m_FramesData[m_pVulkanBackend->m_CurrentFrame].RenderFence, VK_TRUE, UINT64_MAX));
    
    uint32_t ImageIndex;
	VkResult Result = vkAcquireNextImageKHR(m_pVulkanDevice->m_Device, m_pVulkanSwapchain->m_Swapchain, UINT64_MAX, m_pVulkanBackend->m_FramesData[m_pVulkanBackend->m_CurrentFrame].PresentSemaphore, VK_NULL_HANDLE, &ImageIndex);
	if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR || m_pVulkanBackend->m_bWasWindowResized)
	{
		m_pVulkanBackend->m_bWasWindowResized = false;
		m_pVulkanSwapchain->RecreateSwapchain();
		return;
	}
	else if (Result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

    m_pVulkanBackend->UpdateFrameUBO(aCamera, m_pVulkanBackend->m_CurrentFrame);

    // Delay fence reset to prevent possible deadlock when recreating the swapchain.
	VK_CHECK(vkResetFences(m_pVulkanDevice->m_Device, 1, &m_pVulkanBackend->m_FramesData[m_pVulkanBackend->m_CurrentFrame].RenderFence));

	vkResetCommandBuffer(m_pVulkanBackend->m_FramesData[m_pVulkanBackend->m_CurrentFrame].MainCommandBuffer, 0);
	RecordLightPassCommands(m_pVulkanBackend->m_FramesData[m_pVulkanBackend->m_CurrentFrame].MainCommandBuffer, ImageIndex);

	VkSubmitInfo GBuffersSubmit = vkinit::SubmitInfo(&m_DeferredCommandBuffer);
	
    VkSemaphore WaitSemaphores[] = {m_pVulkanBackend->m_FramesData[m_pVulkanBackend->m_CurrentFrame].PresentSemaphore};
	VkPipelineStageFlags GBuffersWaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	GBuffersSubmit.pWaitDstStageMask = &GBuffersWaitStage;
	GBuffersSubmit.waitSemaphoreCount = 1;
	GBuffersSubmit.pWaitSemaphores = WaitSemaphores;
	GBuffersSubmit.signalSemaphoreCount = 1;
	GBuffersSubmit.pSignalSemaphores = &m_GBufferReadySemaphore;

	VK_CHECK(vkQueueSubmit(m_pVulkanDevice->m_GraphicsQueue, 1, &GBuffersSubmit, nullptr));

    VkSubmitInfo RenderSubmit = vkinit::SubmitInfo(&m_pVulkanBackend->m_FramesData[m_pVulkanBackend->m_CurrentFrame].MainCommandBuffer);

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	
    VkSemaphore SignalSemaphores[] = {m_pVulkanBackend->m_FramesData[m_pVulkanBackend->m_CurrentFrame].RenderSemaphore};
	RenderSubmit.pWaitDstStageMask = &waitStage;
	RenderSubmit.waitSemaphoreCount = 1;
	RenderSubmit.pWaitSemaphores = &m_GBufferReadySemaphore;
	RenderSubmit.signalSemaphoreCount = 1;
	RenderSubmit.pSignalSemaphores = SignalSemaphores;

	VK_CHECK(vkQueueSubmit(m_pVulkanDevice->m_GraphicsQueue, 1, &RenderSubmit, m_pVulkanBackend->m_FramesData[m_pVulkanBackend->m_CurrentFrame].RenderFence));

	VkPresentInfoKHR PresentInfo = vkinit::PresentInfo();
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = SignalSemaphores;
	VkSwapchainKHR SwapChains[] = {m_pVulkanSwapchain->m_Swapchain};
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = SwapChains;
	PresentInfo.pImageIndices = &ImageIndex;

	vkQueuePresentKHR(m_pVulkanDevice->m_GraphicsQueue, &PresentInfo);

	m_pVulkanBackend->m_CurrentFrame = (m_pVulkanBackend->m_CurrentFrame + 1) % FRAME_OVERLAP;
}

void CVulkanDeferredRenderPath::UpdateBuffers()
{	
	const auto Camera = CEngine::Get()->GetRenderModule()->GetCamera();

	glm::mat4 Projection = glm::perspective(glm::radians(70.0f), m_pVulkanSwapchain->m_WindowExtent.width / (float)m_pVulkanSwapchain->m_WindowExtent.height, 0.1f, 200.0f);
	Projection[1][1] *= -1;

	sGPUCameraData CameraData;
	// TODO: Do not harcode this.
	CameraData.Projection =  
	CameraData.View = Camera->GetViewMatrix();
	CameraData.Viewproj = Projection * CameraData.View;

	void* Data;
	vmaMapMemory(m_pVulkanDevice->m_Allocator, m_CameraBuffer.Allocation, &Data);
	memcpy(Data, &CameraData, sizeof(sGPUCameraData));
	vmaUnmapMemory(m_pVulkanDevice->m_Allocator, m_CameraBuffer.Allocation);
}

void CVulkanDeferredRenderPath::HandleSceneChanged()
{
	SGSINFO("Handle Scene Changed");
	RecordGBufferPassCommands();
}

void CVulkanDeferredRenderPath::CreateDeferredQuad()
{
	sMeshData Quad = CGeometryGenerator::CreateQuad(-1.0f, 1.0f, 2.0f, 2.0f, 0.0f);
	Quad.ID = "DeferredQuad"; //TODO: Improve this.

	m_Quad.pMesh = new sMesh("DeferredQuad");
	m_Quad.pMesh->NumIndices = static_cast<uint32_t>(Quad.Indices32.size());
	vkutils::CreateVertexBuffer(m_pVulkanDevice, Quad.Vertices, m_Quad.pMesh->VertexBuffer);
	vkutils::CreateIndexBuffer(m_pVulkanDevice, Quad.Indices32, m_Quad.pMesh->IndexBuffer);
}

void CVulkanDeferredRenderPath::CreateDeferredAttachments()
{
    VkExtent3D AttachmentExtent
	{
		m_pVulkanSwapchain->m_WindowExtent.width,
		m_pVulkanSwapchain->m_WindowExtent.height,
		1
	};

	m_PositionFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	m_NormalFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	m_AlbedoFormat = VK_FORMAT_R8G8B8A8_UNORM;

	VkImageCreateInfo PositionImgInfo = vkinit::ImageCreateInfo(m_PositionFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, AttachmentExtent);
	VkImageCreateInfo NormalImgInfo = vkinit::ImageCreateInfo(m_NormalFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, AttachmentExtent);
	VkImageCreateInfo AlbedoImgInfo = vkinit::ImageCreateInfo(m_AlbedoFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, AttachmentExtent);

	VmaAllocationCreateInfo ImgAllocInfo = {};
	ImgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	ImgAllocInfo.requiredFlags = VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(m_pVulkanDevice->m_Allocator, &PositionImgInfo, &ImgAllocInfo, &m_PositionImage.Image, &m_PositionImage.Allocation, nullptr);
	vmaCreateImage(m_pVulkanDevice->m_Allocator, &NormalImgInfo, &ImgAllocInfo, &m_NormalImage.Image, &m_NormalImage.Allocation, nullptr);
	vmaCreateImage(m_pVulkanDevice->m_Allocator, &AlbedoImgInfo, &ImgAllocInfo, &m_AlbedoImage.Image, &m_AlbedoImage.Allocation, nullptr);

	VkImageViewCreateInfo PositionViewInfo = vkinit::ImageViewCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, m_PositionImage.Image, VK_IMAGE_ASPECT_COLOR_BIT);
	VkImageViewCreateInfo NormalViewInfo = vkinit::ImageViewCreateInfo(VK_FORMAT_R16G16B16A16_SFLOAT, m_NormalImage.Image, VK_IMAGE_ASPECT_COLOR_BIT);
	VkImageViewCreateInfo AlbedoViewInfo = vkinit::ImageViewCreateInfo(VK_FORMAT_R8G8B8A8_UNORM, m_AlbedoImage.Image, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_CHECK(vkCreateImageView(m_pVulkanDevice->m_Device, &PositionViewInfo, nullptr, &m_PositionImageView));
	VK_CHECK(vkCreateImageView(m_pVulkanDevice->m_Device, &NormalViewInfo, nullptr, &m_NormalImageView));
	VK_CHECK(vkCreateImageView(m_pVulkanDevice->m_Device, &AlbedoViewInfo, nullptr, &m_AlbedoImageView));

	m_MainDeletionQueue.PushFunction([=]()
	{
		vkDestroyImageView(m_pVulkanDevice->m_Device, m_PositionImageView, nullptr);
		vmaDestroyImage(m_pVulkanDevice->m_Allocator, m_PositionImage.Image, m_PositionImage.Allocation);
		vkDestroyImageView(m_pVulkanDevice->m_Device, m_NormalImageView, nullptr);
		vmaDestroyImage(m_pVulkanDevice->m_Allocator, m_NormalImage.Image, m_NormalImage.Allocation);
		vkDestroyImageView(m_pVulkanDevice->m_Device, m_AlbedoImageView, nullptr);
		vmaDestroyImage(m_pVulkanDevice->m_Allocator, m_AlbedoImage.Image, m_AlbedoImage.Allocation);
	});
}

void CVulkanDeferredRenderPath::CreateGBufferDescriptors()
{
    VkDescriptorPoolSize PoolSize = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 };

	VkDescriptorPoolCreateInfo PoolInfo = {};
	PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	PoolInfo.flags = 0;
	PoolInfo.maxSets = 10;
	PoolInfo.poolSizeCount = 1;
	PoolInfo.pPoolSizes = &PoolSize;

	vkCreateDescriptorPool(m_pVulkanDevice->m_Device, &PoolInfo, nullptr, &m_DeferredDescriptorPool);

	//gbuffers
	VkDescriptorSetLayoutBinding PositionBind = vkinit::DescriptorLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	VkDescriptorSetLayoutBinding NormalBind = vkinit::DescriptorLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	VkDescriptorSetLayoutBinding AlbedoBind = vkinit::DescriptorLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2);

	std::array<VkDescriptorSetLayoutBinding, 3> DeferredSetLayouts = { PositionBind, NormalBind, AlbedoBind };

	VkDescriptorSetLayoutCreateInfo DeferredLayoutInfo = {};
	DeferredLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DeferredLayoutInfo.pNext = nullptr;
	DeferredLayoutInfo.flags = 0;
	DeferredLayoutInfo.bindingCount = static_cast<uint32_t>(DeferredSetLayouts.size());
	DeferredLayoutInfo.pBindings = DeferredSetLayouts.data();

	VK_CHECK(vkCreateDescriptorSetLayout(m_pVulkanDevice->m_Device, &DeferredLayoutInfo, nullptr, &m_GBufferSetLayout));

	VkDescriptorSetAllocateInfo DeferredSetAlloc = {};
	DeferredSetAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	DeferredSetAlloc.pNext = nullptr;
	DeferredSetAlloc.descriptorPool = m_DeferredDescriptorPool;
	DeferredSetAlloc.descriptorSetCount = 1;
	DeferredSetAlloc.pSetLayouts = &m_GBufferSetLayout;

	vkAllocateDescriptorSets(m_pVulkanDevice->m_Device, &DeferredSetAlloc, &m_GBufferDescriptorSet);

	VkDescriptorImageInfo PositionDescriptorImage;
	PositionDescriptorImage.sampler = m_pVulkanBackend->m_DefaultSampler;
	PositionDescriptorImage.imageView = m_PositionImageView;
	PositionDescriptorImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkDescriptorImageInfo NormalDescriptorImage;
	NormalDescriptorImage.sampler = m_pVulkanBackend->m_DefaultSampler;
	NormalDescriptorImage.imageView = m_NormalImageView;
	NormalDescriptorImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkDescriptorImageInfo AlbedoDescriptorImage;
	AlbedoDescriptorImage.sampler = m_pVulkanBackend->m_DefaultSampler;
	AlbedoDescriptorImage.imageView = m_AlbedoImageView;
	AlbedoDescriptorImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet PositionTextureWrite = vkinit::WriteDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_GBufferDescriptorSet, &PositionDescriptorImage, 0);
	VkWriteDescriptorSet NormalTextureWrite = vkinit::WriteDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_GBufferDescriptorSet, &NormalDescriptorImage, 1);
	VkWriteDescriptorSet AlbedoTextureWrite = vkinit::WriteDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_GBufferDescriptorSet, &AlbedoDescriptorImage, 2);

	std::array<VkWriteDescriptorSet, 3> SetWrites = { PositionTextureWrite, NormalTextureWrite, AlbedoTextureWrite };

	vkUpdateDescriptorSets(m_pVulkanDevice->m_Device, static_cast<uint32_t>(SetWrites.size()), SetWrites.data(), 0, nullptr);

	VkDescriptorSetLayoutBinding cameraBind = vkinit::DescriptorLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

	VkDescriptorSetLayoutCreateInfo CamSetInfo = {};
	CamSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	CamSetInfo.pNext = nullptr;

	CamSetInfo.bindingCount = 1;
	CamSetInfo.flags = 0;
	CamSetInfo.pBindings = &cameraBind;

	vkCreateDescriptorSetLayout(m_pVulkanDevice->m_Device, &CamSetInfo, nullptr, &m_CameraSetLayout);

	VkDescriptorSetAllocateInfo CameraSetAllocInfo = {};
	CameraSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	CameraSetAllocInfo.pNext = nullptr;
	CameraSetAllocInfo.descriptorPool = m_DeferredDescriptorPool;
	CameraSetAllocInfo.descriptorSetCount = 1;
	CameraSetAllocInfo.pSetLayouts = &m_CameraSetLayout;
	
	vkAllocateDescriptorSets(m_pVulkanDevice->m_Device, &CameraSetAllocInfo, &m_CameraDescriptorSet);

	m_CameraBuffer = vkutils::CreateBuffer(m_pVulkanDevice, sizeof(sGPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	VkDescriptorBufferInfo CamBufferInfo = {};
	CamBufferInfo.buffer = m_CameraBuffer.Buffer;
	CamBufferInfo.offset = 0;
	CamBufferInfo.range = sizeof(sGPUCameraData);

	VkWriteDescriptorSet CamWrite = vkinit::WriteDescriptorBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_CameraDescriptorSet, &CamBufferInfo, 0);

	vkUpdateDescriptorSets(m_pVulkanDevice->m_Device, 1, &CamWrite, 0, nullptr);

	m_MainDeletionQueue.PushFunction([=]
	{
		vkDestroyDescriptorSetLayout(m_pVulkanDevice->m_Device, m_CameraSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_pVulkanDevice->m_Device, m_GBufferSetLayout, nullptr);
		vkDestroyDescriptorPool(m_pVulkanDevice->m_Device, m_DeferredDescriptorPool, nullptr);
	});
}

void CVulkanDeferredRenderPath::CreateDeferredRenderPass()
{
	// G-Buffers Pass.
	VkAttachmentDescription PositionAttachment = {};
	PositionAttachment.format = m_PositionFormat;
	PositionAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	PositionAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	PositionAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	PositionAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	PositionAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	PositionAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	PositionAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentDescription NormalAttachment = {};
	NormalAttachment.format = m_NormalFormat;
	NormalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	NormalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	NormalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	NormalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	NormalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	NormalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	NormalAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentDescription AlbedoAttachment = {};
	AlbedoAttachment.format = m_AlbedoFormat;
	AlbedoAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	AlbedoAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	AlbedoAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	AlbedoAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	AlbedoAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	AlbedoAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	AlbedoAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentDescription DepthAttachment = {};
	DepthAttachment.format = m_pVulkanDevice->FindDepthFormat();
	DepthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	DepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	DepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	DepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	DepthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::array<VkAttachmentDescription, 4> AttachmentDescriptions = { PositionAttachment, NormalAttachment, AlbedoAttachment, DepthAttachment };

	VkAttachmentReference PositionRef;
	PositionRef.attachment = 0;
	PositionRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference NormalRef;
	NormalRef.attachment = 1;
	NormalRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference AlbedoRef;
	AlbedoRef.attachment = 2;
	AlbedoRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	std::array<VkAttachmentReference, 3> ColorReferences = { PositionRef, NormalRef, AlbedoRef };

	VkAttachmentReference DepthRef;
	DepthRef.attachment = 3;
	DepthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription Subpass = {};
	Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	Subpass.colorAttachmentCount = static_cast<uint32_t>(ColorReferences.size());
	Subpass.pColorAttachments = ColorReferences.data();
	Subpass.pDepthStencilAttachment = &DepthRef;

	std::array<VkSubpassDependency, 2> Dependencies;

	Dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	Dependencies[0].dstSubpass = 0;
	Dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	Dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	Dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	Dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	Dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	Dependencies[1].srcSubpass = 0;
	Dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	Dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	Dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	Dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	Dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	Dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo DeferredPass = {};
	DeferredPass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	DeferredPass.pNext = nullptr;
	DeferredPass.attachmentCount = static_cast<uint32_t>(AttachmentDescriptions.size());
	DeferredPass.pAttachments = AttachmentDescriptions.data();
	DeferredPass.subpassCount = 1;
	DeferredPass.pSubpasses = &Subpass;
	DeferredPass.dependencyCount = static_cast<uint32_t>(Dependencies.size());
	DeferredPass.pDependencies = Dependencies.data();

	VK_CHECK(vkCreateRenderPass(m_pVulkanDevice->m_Device, &DeferredPass, nullptr, &m_DeferredRenderPass));

	m_MainDeletionQueue.PushFunction([=]() {
		vkDestroyRenderPass(m_pVulkanDevice->m_Device, m_DeferredRenderPass, nullptr);
		});
}

void CVulkanDeferredRenderPath::CreateGBufferFramebuffer()
{
	std::array<VkImageView, 4> Attachments;
	Attachments[0] = m_PositionImageView;
	Attachments[1] = m_NormalImageView;
	Attachments[2] = m_AlbedoImageView;
	Attachments[3] = m_pVulkanSwapchain->m_DepthImageView;

	VkFramebufferCreateInfo FramebufferInfo = {};
	FramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	FramebufferInfo.pNext = nullptr;
	FramebufferInfo.renderPass = m_DeferredRenderPass;
	FramebufferInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
	FramebufferInfo.pAttachments = Attachments.data();
	FramebufferInfo.width = m_pVulkanSwapchain->m_WindowExtent.width;
	FramebufferInfo.height = m_pVulkanSwapchain->m_WindowExtent.height;
	FramebufferInfo.layers = 1;

	VK_CHECK(vkCreateFramebuffer(m_pVulkanDevice->m_Device, &FramebufferInfo, nullptr, &m_GBufferFramebuffer));

	m_MainDeletionQueue.PushFunction([=]
	{
		vkDestroyFramebuffer(m_pVulkanDevice->m_Device, m_GBufferFramebuffer, nullptr);
	});
}

void CVulkanDeferredRenderPath::CreateDeferredPipeline()
{
	VkShaderModule DeferredVertShader;
	// TODO: Do not hardcode this.
	if (!vkutils::LoadShaderModule(m_pVulkanDevice->m_Device, "../Engine/shaders/deferred_vert.spv", &DeferredVertShader))
	{
		std::cout << "Error when building the deferred vertex shader module" << std::endl;
	}
	else
	{
		std::cout << "Deferred Vertex Shader loaded SUCCESSFULLY!" << std::endl;
	}

	VkShaderModule DeferredFragShader;
	if (!vkutils::LoadShaderModule(m_pVulkanDevice->m_Device, "../Engine/shaders/deferred_frag.spv", &DeferredFragShader))
	{
		std::cout << "Error when building the deferred fragment shader module" << std::endl;
	}
	else
	{
		std::cout << "Deferred Fragment Shader loaded SUCCESSFULLY!" << std::endl;
	}

	VkShaderModule LightVertexShader;
	if (!vkutils::LoadShaderModule(m_pVulkanDevice->m_Device, "../Engine/shaders/light_vert.spv", &LightVertexShader))
	{
		std::cout << "Error when building the light vertex shader module" << std::endl;
	}
	else
	{
		std::cout << "Light Vertex Shader loaded SUCCESSFULLY!" << std::endl;
	}

	VkShaderModule LightFragShader;
	if (!vkutils::LoadShaderModule(m_pVulkanDevice->m_Device, "../Engine/shaders/light_frag.spv", &LightFragShader))
	{
		std::cout << "Error when building the light fragment shader module" << std::endl;
	}
	else
	{
		std::cout << "Light Fragment Shader loaded SUCCESSFULLY!" << std::endl;
	}

	// Layouts
	VkPipelineLayoutCreateInfo LayoutInfo = vkinit::PipelineLayoutCreateInfo();

	std::array<VkDescriptorSetLayout, 2> DeferredSetLayouts = { m_CameraSetLayout, m_pVulkanBackend->m_RenderObjectsSetLayout };

	LayoutInfo.pushConstantRangeCount = 0;
	LayoutInfo.pPushConstantRanges = nullptr;
	LayoutInfo.setLayoutCount = static_cast<uint32_t>(DeferredSetLayouts.size());
	LayoutInfo.pSetLayouts = DeferredSetLayouts.data();

	VK_CHECK(vkCreatePipelineLayout(m_pVulkanDevice->m_Device, &LayoutInfo, nullptr, &m_DeferredPipelineLayout));

	std::array<VkDescriptorSetLayout, 2> LightSetLayouts = { m_pVulkanBackend->m_DescriptorSetLayout, m_GBufferSetLayout };

	LayoutInfo.setLayoutCount = static_cast<uint32_t>(LightSetLayouts.size());
	LayoutInfo.pSetLayouts = LightSetLayouts.data();

	VK_CHECK(vkCreatePipelineLayout(m_pVulkanDevice->m_Device, &LayoutInfo, nullptr, &m_LightPipelineLayout));


	// G-Buffers Pipeline Creation.
	PipelineBuilder PipelineBuilder;

	std::vector<VkDynamicState> DynamicStates = 
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	PipelineBuilder.m_DynamicState = vkinit::DynamicStateCreateInfo(DynamicStates);

	sVertexInputDescription VertexDescription = GetVertexDescription();

	PipelineBuilder.m_VertexInputInfo = vkinit::VertexInputStateCreateInfo();
	PipelineBuilder.m_VertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(VertexDescription.Bindings.size());
	PipelineBuilder.m_VertexInputInfo.pVertexBindingDescriptions = VertexDescription.Bindings.data();
	PipelineBuilder.m_VertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(VertexDescription.Attributes.size());
	PipelineBuilder.m_VertexInputInfo.pVertexAttributeDescriptions = VertexDescription.Attributes.data();

	PipelineBuilder.m_InputAssembly = vkinit::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	PipelineBuilder.m_Viewport.x = 0.0f;
	PipelineBuilder.m_Viewport.y = 0.0f;
	PipelineBuilder.m_Viewport.width = static_cast<float>(m_pVulkanSwapchain->m_WindowExtent.width);
	PipelineBuilder.m_Viewport.height = static_cast<float>(m_pVulkanSwapchain->m_WindowExtent.height);
	PipelineBuilder.m_Viewport.minDepth = 0.0f;
	PipelineBuilder.m_Viewport.maxDepth = 1.0f;

	PipelineBuilder.m_Scissor.offset = {0, 0};
	PipelineBuilder.m_Scissor.extent = m_pVulkanSwapchain->m_WindowExtent;

	PipelineBuilder.m_DepthStencil = vkinit::DepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS);
	PipelineBuilder.m_Rasterizer = vkinit::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
	PipelineBuilder.m_Multisampling = vkinit::MultisamplingStateCreateInfo();

	PipelineBuilder.m_ColorBlendAttachment.push_back(vkinit::ColorBlendAttachmentState());
	PipelineBuilder.m_ColorBlendAttachment.push_back(vkinit::ColorBlendAttachmentState());
	PipelineBuilder.m_ColorBlendAttachment.push_back(vkinit::ColorBlendAttachmentState());

	//Deferred Shaders
	PipelineBuilder.m_ShaderStages.push_back(
		vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, DeferredVertShader));
	PipelineBuilder.m_ShaderStages.push_back(
		vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, DeferredFragShader));

	PipelineBuilder.m_PipelineLayout = m_DeferredPipelineLayout;

	m_DeferredPipeline = PipelineBuilder.BuildPipeline(m_pVulkanDevice->m_Device, m_DeferredRenderPass);

	// Light Pipeline Creation.

	//Light Depth Stencil
	PipelineBuilder.m_DepthStencil = vkinit::DepthStencilCreateInfo(false, false, VK_COMPARE_OP_ALWAYS);

	//Deferred Color Blend Attachment
	PipelineBuilder.m_ColorBlendAttachment.clear();
	PipelineBuilder.m_ColorBlendAttachment.push_back(vkinit::ColorBlendAttachmentState());

	//Light Shaders
	PipelineBuilder.m_ShaderStages.clear();
	PipelineBuilder.m_ShaderStages.push_back(
		vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, LightVertexShader));
	PipelineBuilder.m_ShaderStages.push_back(
		vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, LightFragShader));

	//Light Layout
	PipelineBuilder.m_PipelineLayout = m_LightPipelineLayout;

	m_LightPipeline = PipelineBuilder.BuildPipeline(m_pVulkanDevice->m_Device, m_pVulkanSwapchain->m_RenderPass);

	//DELETIONS

	vkDestroyShaderModule(m_pVulkanDevice->m_Device, LightFragShader, nullptr);
	vkDestroyShaderModule(m_pVulkanDevice->m_Device, LightVertexShader, nullptr);
	vkDestroyShaderModule(m_pVulkanDevice->m_Device, DeferredFragShader, nullptr);
	vkDestroyShaderModule(m_pVulkanDevice->m_Device, DeferredVertShader, nullptr);

	m_MainDeletionQueue.PushFunction([=]() {
		vkDestroyPipeline(m_pVulkanDevice->m_Device, m_DeferredPipeline, nullptr);
		vkDestroyPipeline(m_pVulkanDevice->m_Device, m_LightPipeline, nullptr);

		vkDestroyPipelineLayout(m_pVulkanDevice->m_Device, m_DeferredPipelineLayout, nullptr);
		vkDestroyPipelineLayout(m_pVulkanDevice->m_Device, m_LightPipelineLayout, nullptr);
		});
}

void CVulkanDeferredRenderPath::CreateDeferredCommandStructures()
{
    VkCommandPoolCreateInfo CommandPoolInfo = vkinit::CommandPoolCreateInfo(m_pVulkanDevice->m_GraphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	
	VK_CHECK(vkCreateCommandPool(m_pVulkanDevice->m_Device, &CommandPoolInfo, nullptr, &m_DeferredCommandPool));

	VkCommandBufferAllocateInfo CmdAllocInfo = vkinit::CommandBufferAllocateInfo(m_DeferredCommandPool, 1);
	VK_CHECK(vkAllocateCommandBuffers(m_pVulkanDevice->m_Device, &CmdAllocInfo, &m_DeferredCommandBuffer));

	m_MainDeletionQueue.PushFunction([=]() {
		vkDestroyCommandPool(m_pVulkanDevice->m_Device, m_DeferredCommandPool, nullptr);
	});
}

void CVulkanDeferredRenderPath::CreateDeferredSyncrhonizationStructures()
{
	VkSemaphoreCreateInfo GBufferReadySempahoreInfo = {};
	GBufferReadySempahoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	GBufferReadySempahoreInfo.pNext = nullptr;
	GBufferReadySempahoreInfo.flags = 0;

	VK_CHECK(vkCreateSemaphore(m_pVulkanDevice->m_Device, &GBufferReadySempahoreInfo, nullptr, &m_GBufferReadySemaphore));

	m_MainDeletionQueue.PushFunction([=]() {
		vkDestroySemaphore(m_pVulkanDevice->m_Device, m_GBufferReadySemaphore, nullptr);
	});
}
