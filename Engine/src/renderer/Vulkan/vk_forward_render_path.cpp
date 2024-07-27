#include "vk_forward_render_path.hpp"
#include "vulkan_backend.hpp"
#include "vulkan_device.hpp"
#include "vulkan_swapchain.hpp"
#include "vk_initializers.hpp"
#include "vk_utils.hpp"

#include <iostream>
#include <array>

CVulkanForwardRenderPath::CVulkanForwardRenderPath(CVulkanBackend* apVulkanBackend, CVulkanDevice* apVulkanDevice, CVulkanSwapchain* apVulkanSwapchain) :
    m_pVulkanBackend(apVulkanBackend), m_pVulkanDevice(apVulkanDevice), m_pVulkanSwapchain(apVulkanSwapchain)
{
}

void CVulkanForwardRenderPath::CreateResources()
{
    CreateForwardPipeline();
}

void CVulkanForwardRenderPath::DestroyResources()
{
	vkQueueWaitIdle(m_pVulkanDevice->m_GraphicsQueue);
    m_MainDeletionQueue.Flush();
}

void CVulkanForwardRenderPath::RecordCommands(VkCommandBuffer aCommandBuffer, uint32_t aImageIdx)
{
	VkCommandBufferBeginInfo BeginInfo = vkinit::CommandBufferBeginInfo();

	VK_CHECK(vkBeginCommandBuffer(aCommandBuffer, &BeginInfo));

	VkRenderPassBeginInfo RenderPassInfo = vkinit::RenderPassBeginInfo(m_pVulkanSwapchain->m_RenderPass, m_pVulkanSwapchain->m_WindowExtent, m_pVulkanSwapchain->m_Framebuffers[aImageIdx]);

	std::array<VkClearValue, 2> ClearValues = {};
	ClearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
	ClearValues[1].depthStencil = {1.0f, 0};
	
	RenderPassInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
	RenderPassInfo.pClearValues = ClearValues.data();

	vkCmdBeginRenderPass(aCommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ForwardPipeline);

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

	VkDeviceSize Offset = 0;
	for (size_t i = 0; i < m_pVulkanBackend->m_RenderObjectsData.size(); ++i)
	{
		const std::array<VkDescriptorSet, 3> DescriptorSets = 
			{ m_pVulkanBackend->m_FramesData[aImageIdx].DescriptorSet, m_pVulkanBackend->m_ObjectsDataDescriptorSet, m_pVulkanBackend->m_RenderObjectsData[i].MaterialDescriptor->DescriptorSet };
		
		vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ForwardPipelineLayout, 
			0, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data(), 0, nullptr);

		vkCmdBindVertexBuffers(aCommandBuffer, 0, 1, &m_pVulkanBackend->m_RenderObjectsData[i].pMesh->VertexBuffer.Buffer, &Offset);
		vkCmdBindIndexBuffer(aCommandBuffer, m_pVulkanBackend->m_RenderObjectsData[i].pMesh->IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);
		// TODO: Batch rendering.
		vkCmdDrawIndexed(aCommandBuffer, m_pVulkanBackend->m_RenderObjectsData[i].pMesh->NumIndices, 1, 0, 0, i);
	}

	vkCmdEndRenderPass(aCommandBuffer);

	VK_CHECK(vkEndCommandBuffer(aCommandBuffer));
}

void CVulkanForwardRenderPath::Render(const CCamera* const aCamera)
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
	RecordCommands(m_pVulkanBackend->m_FramesData[m_pVulkanBackend->m_CurrentFrame].MainCommandBuffer, ImageIndex);

	VkSubmitInfo SubmitInfo = vkinit::SubmitInfo(&m_pVulkanBackend->m_FramesData[m_pVulkanBackend->m_CurrentFrame].MainCommandBuffer);

	VkSemaphore WaitSemaphores[] = {m_pVulkanBackend->m_FramesData[m_pVulkanBackend->m_CurrentFrame].PresentSemaphore};
	VkPipelineStageFlags WaitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	SubmitInfo.waitSemaphoreCount = 1;
	SubmitInfo.pWaitSemaphores = WaitSemaphores;
	SubmitInfo.pWaitDstStageMask = WaitStages;

	VkSemaphore SignalSemaphores[] = {m_pVulkanBackend->m_FramesData[m_pVulkanBackend->m_CurrentFrame].RenderSemaphore};
	SubmitInfo.signalSemaphoreCount = 1;
	SubmitInfo.pSignalSemaphores = SignalSemaphores;

	VK_CHECK(vkQueueSubmit(m_pVulkanDevice->m_GraphicsQueue, 1, &SubmitInfo, m_pVulkanBackend->m_FramesData[m_pVulkanBackend->m_CurrentFrame].RenderFence));

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

void CVulkanForwardRenderPath::HandleSceneChanged()
{
}

void CVulkanForwardRenderPath::CreateForwardPipeline()
{
	VkShaderModule VertShader;
	// TODO: Do not hardcode this.
	if (!vkutils::LoadShaderModule(m_pVulkanDevice->m_Device, "../Engine/shaders/vert.spv", &VertShader))
	{
		std::cout << "Error when building the vertex shader module" << std::endl;
	}
	else
	{
		std::cout << "Vertex Shader loaded SUCCESSFULLY!" << std::endl;
	}

	VkShaderModule FragShader;
	if (!vkutils::LoadShaderModule(m_pVulkanDevice->m_Device, "../Engine/shaders/frag.spv", &FragShader))
	{
		std::cout << "Error when building the fragment shader module" << std::endl;
	}
	else
	{
		std::cout << "Fragment Shader loaded SUCCESSFULLY!" << std::endl;
	}

	VkPipelineLayoutCreateInfo PipelineLayoutInfo = vkinit::PipelineLayoutCreateInfo();
	std::array<VkDescriptorSetLayout, 3> SetLayouts = { m_pVulkanBackend->m_DescriptorSetLayout, m_pVulkanBackend->m_RenderObjectsSetLayout, m_pVulkanBackend->m_MaterialsSetLayout };
	PipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(SetLayouts.size());
	PipelineLayoutInfo.pSetLayouts = SetLayouts.data();

	VK_CHECK(vkCreatePipelineLayout(m_pVulkanDevice->m_Device, &PipelineLayoutInfo, nullptr, &m_ForwardPipelineLayout));

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
	PipelineBuilder.m_Rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	PipelineBuilder.m_Rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	PipelineBuilder.m_Multisampling = vkinit::MultisamplingStateCreateInfo();
	PipelineBuilder.m_ColorBlendAttachment.push_back(vkinit::ColorBlendAttachmentState());

	PipelineBuilder.m_ShaderStages.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, VertShader));
	PipelineBuilder.m_ShaderStages.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, FragShader));

	PipelineBuilder.m_PipelineLayout = m_ForwardPipelineLayout;

	m_ForwardPipeline = PipelineBuilder.BuildPipeline(m_pVulkanDevice->m_Device, m_pVulkanSwapchain->m_RenderPass);

	vkDestroyShaderModule(m_pVulkanDevice->m_Device, VertShader, nullptr);
	vkDestroyShaderModule(m_pVulkanDevice->m_Device, FragShader, nullptr);

	m_MainDeletionQueue.PushFunction([=]()
	{
		vkDestroyPipeline(m_pVulkanDevice->m_Device, m_ForwardPipeline, nullptr);
		vkDestroyPipelineLayout(m_pVulkanDevice->m_Device, m_ForwardPipelineLayout, nullptr);
	});
}
