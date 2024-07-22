#include "vk_initializers.hpp"
#include <iostream>

VkCommandPoolCreateInfo vkinit::CommandPoolCreateInfo(uint32 aQueueFamilyIndex, VkCommandPoolCreateFlags aFlags)
{
    VkCommandPoolCreateInfo CreateInfo = {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    CreateInfo.pNext = nullptr;
    CreateInfo.queueFamilyIndex = aQueueFamilyIndex;
    CreateInfo.flags = aFlags;

    return CreateInfo;
}

VkCommandBufferAllocateInfo vkinit::CommandBufferAllocateInfo(VkCommandPool aCommandPool, uint32 aCount, VkCommandBufferLevel aLevel)
{
    VkCommandBufferAllocateInfo CreateInfo = {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CreateInfo.pNext = nullptr;
    CreateInfo.commandPool = aCommandPool;
    CreateInfo.commandBufferCount = aCount;
    CreateInfo.level = aLevel;

    return CreateInfo;
}
	
VkDescriptorSetLayoutBinding vkinit::DescriptorLayoutBinding(VkDescriptorType aType, VkShaderStageFlags aStageFlags, uint32_t aBinding)
{
	VkDescriptorSetLayoutBinding SetBind = {};
	SetBind.binding = aBinding;
	SetBind.descriptorCount = 1;
	SetBind.descriptorType = aType;
	SetBind.pImmutableSamplers = nullptr;
	SetBind.stageFlags = aStageFlags;

	return SetBind;
}

VkWriteDescriptorSet vkinit::WriteDescriptorBuffer(VkDescriptorType aType, VkDescriptorSet aDstSet, VkDescriptorBufferInfo* aBufferInfo, uint32_t aBinding, uint32_t aCount)
{
	VkWriteDescriptorSet Write = {};
	Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	Write.pNext = nullptr;

	Write.dstBinding = aBinding;
	Write.dstSet = aDstSet;
	Write.descriptorCount = aCount;
	Write.descriptorType = aType;
	Write.pBufferInfo = aBufferInfo;

	return Write;
}

VkWriteDescriptorSet vkinit::WriteDescriptorImage(VkDescriptorType aType, VkDescriptorSet aDstSet, VkDescriptorImageInfo* aImageInfo, uint32_t aBinding)
{
	VkWriteDescriptorSet Write = {};
	Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	Write.pNext = nullptr;

	Write.dstBinding = aBinding;
	Write.dstSet = aDstSet;
	Write.descriptorCount = 1;
	Write.descriptorType = aType;
	Write.pImageInfo = aImageInfo;

	return Write;
}

VkPipelineLayoutCreateInfo vkinit::PipelineLayoutCreateInfo()
{
    VkPipelineLayoutCreateInfo Info{};
	Info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	Info.pNext = nullptr;

	//empty defaults
	Info.flags = 0;
	Info.setLayoutCount = 0;
	Info.pSetLayouts = nullptr;
	Info.pushConstantRangeCount = 0;
	Info.pPushConstantRanges = nullptr;
	return Info;
}

VkPipelineDynamicStateCreateInfo vkinit::DynamicStateCreateInfo(const std::vector<VkDynamicState>& aDynamicStates)
{
    VkPipelineDynamicStateCreateInfo DynamicState{};
    DynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicState.dynamicStateCount = static_cast<uint32_t>(aDynamicStates.size());
    DynamicState.pDynamicStates = aDynamicStates.data();

    return DynamicState;
}

VkPipelineVertexInputStateCreateInfo vkinit::VertexInputStateCreateInfo()
{
	VkPipelineVertexInputStateCreateInfo Info = {};
	Info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	Info.pNext = nullptr;

	//no vertex bindings or attributes
	Info.vertexBindingDescriptionCount = 0;
	Info.vertexAttributeDescriptionCount = 0;
	return Info;
}

VkPipelineInputAssemblyStateCreateInfo vkinit::InputAssemblyCreateInfo(VkPrimitiveTopology aTopology)
{
    VkPipelineInputAssemblyStateCreateInfo Info = {};
	Info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	Info.pNext = nullptr;
	Info.topology = aTopology;
	Info.primitiveRestartEnable = VK_FALSE;
	return Info;
}

VkPipelineDepthStencilStateCreateInfo vkinit::DepthStencilCreateInfo(bool abDepthTest, bool abDepthWrite, VkCompareOp aCompareOp)
{
	VkPipelineDepthStencilStateCreateInfo Info = {};
	Info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	Info.pNext = nullptr;

	Info.depthTestEnable = abDepthTest ? VK_TRUE : VK_FALSE;
	Info.depthWriteEnable = abDepthWrite ? VK_TRUE : VK_FALSE;
	Info.depthCompareOp = abDepthTest ? aCompareOp : VK_COMPARE_OP_ALWAYS;
	Info.depthBoundsTestEnable = VK_FALSE;
	Info.minDepthBounds = 0.0f; // Optional.
	Info.maxDepthBounds = 1.0f; // Optional.
	Info.stencilTestEnable = VK_FALSE;

	return Info;
}

VkPipelineRasterizationStateCreateInfo vkinit::RasterizationStateCreateInfo(VkPolygonMode aPolygonMode)
{
	VkPipelineRasterizationStateCreateInfo Info = {};
	Info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	Info.pNext = nullptr;

	Info.depthClampEnable = VK_FALSE;
	//rasterizer discard allows objects with holes, default to no
	Info.rasterizerDiscardEnable = VK_FALSE;

	Info.polygonMode = aPolygonMode;
	Info.lineWidth = 1.0f;
	//no backface cull
	Info.cullMode = VK_CULL_MODE_NONE;
	Info.frontFace = VK_FRONT_FACE_CLOCKWISE;
	//no depth bias
	Info.depthBiasEnable = VK_FALSE;
	Info.depthBiasConstantFactor = 0.0f;
	Info.depthBiasClamp = 0.0f;
	Info.depthBiasSlopeFactor = 0.0f;

	return Info;
}

VkPipelineMultisampleStateCreateInfo vkinit::MultisamplingStateCreateInfo()
{
	VkPipelineMultisampleStateCreateInfo Info = {};
	Info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	Info.pNext = nullptr;

	Info.sampleShadingEnable = VK_FALSE;
	//multisampling defaulted to no multisampling (1 sample per pixel)
	Info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	Info.minSampleShading = 1.0f;
	Info.pSampleMask = nullptr;
	Info.alphaToCoverageEnable = VK_FALSE;
	Info.alphaToOneEnable = VK_FALSE;
	return Info;
}

VkPipelineColorBlendAttachmentState vkinit::ColorBlendAttachmentState()
{
	VkPipelineColorBlendAttachmentState ColorBlendAttachment = {};
	ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	ColorBlendAttachment.blendEnable = VK_FALSE;
	return ColorBlendAttachment;
}

VkPipelineShaderStageCreateInfo vkinit::PipelineShaderStageCreateInfo(VkShaderStageFlagBits aStage, VkShaderModule aShaderModule)
{
    VkPipelineShaderStageCreateInfo Info{};
	Info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	Info.pNext = nullptr;

	//shader stage
	Info.stage = aStage;
	//module containing the code for this shader stage
	Info.module = aShaderModule;
	//the entry point of the shader
	Info.pName = "main";
	return Info;
}

VkCommandBufferBeginInfo vkinit::CommandBufferBeginInfo(VkCommandBufferUsageFlags aFlags)
{
	VkCommandBufferBeginInfo Info = {};
	Info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	Info.pNext = nullptr;
	
	Info.pInheritanceInfo = nullptr;
	Info.flags = aFlags;
	return Info;
}

VkRenderPassBeginInfo vkinit::RenderPassBeginInfo(VkRenderPass aRenderPass, VkExtent2D aWindowExtent, VkFramebuffer aFramebuffer)
{
    VkRenderPassBeginInfo Info = {};
	Info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	Info.pNext = nullptr;

	Info.renderPass = aRenderPass;
	Info.renderArea.offset.x = 0;
	Info.renderArea.offset.y = 0;
	Info.renderArea.extent = aWindowExtent;
	Info.clearValueCount = 1;
	Info.pClearValues = nullptr;
	Info.framebuffer = aFramebuffer;

	return Info;
}

VkFenceCreateInfo vkinit::FenceCreateInfo(VkFenceCreateFlags aFlags)
{
	VkFenceCreateInfo Info = {};
	Info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	Info.pNext = nullptr;

	Info.flags = aFlags;

	return Info;
}

VkSemaphoreCreateInfo vkinit::SemaphoreCreateInfo(VkSemaphoreCreateFlags aFlags)
{
	VkSemaphoreCreateInfo Info = {};
	Info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	Info.pNext = nullptr;
	Info.flags = aFlags;
	return Info;
}

VkSubmitInfo vkinit::SubmitInfo(VkCommandBuffer* aCmd)
{
	VkSubmitInfo Info = {};
	Info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	Info.pNext = nullptr;

	Info.waitSemaphoreCount = 0;
	Info.pWaitSemaphores = nullptr;
	Info.pWaitDstStageMask = nullptr;
	Info.commandBufferCount = 1;
	Info.pCommandBuffers = aCmd;
	Info.signalSemaphoreCount = 0;
	Info.pSignalSemaphores = nullptr;

	return Info;
}

VkPresentInfoKHR vkinit::PresentInfo()
{
	VkPresentInfoKHR Info = {};
	Info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	Info.pNext = nullptr;

	Info.swapchainCount = 0;
	Info.pSwapchains = nullptr;
	Info.pWaitSemaphores = nullptr;
	Info.waitSemaphoreCount = 0;
	Info.pImageIndices = nullptr;

	return Info;
}

VkImageCreateInfo vkinit::ImageCreateInfo(VkFormat aFormat, VkImageUsageFlags aUsageFlags, VkExtent3D aExtent)
{
	VkImageCreateInfo Info = { };
	Info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	Info.pNext = nullptr;

	Info.imageType = VK_IMAGE_TYPE_2D;

	Info.format = aFormat;
	Info.extent = aExtent;

	Info.mipLevels = 1;
	Info.arrayLayers = 1;
	Info.samples = VK_SAMPLE_COUNT_1_BIT;
	Info.tiling = VK_IMAGE_TILING_OPTIMAL;
	Info.usage = aUsageFlags;

	return Info;
}

VkImageViewCreateInfo vkinit::ImageViewCreateInfo(VkFormat aFormat, VkImage aImage, VkImageAspectFlags aAspectFlags)
{
	VkImageViewCreateInfo Info = {};
	Info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	Info.pNext = nullptr;

	Info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	Info.image = aImage;
	Info.format = aFormat;
	Info.subresourceRange.baseMipLevel = 0;
	Info.subresourceRange.levelCount = 1;
	Info.subresourceRange.baseArrayLayer = 0;
	Info.subresourceRange.layerCount = 1;
	Info.subresourceRange.aspectMask = aAspectFlags;

	return Info;
}

VkPipeline PipelineBuilder::BuildPipeline(VkDevice aDevice, VkRenderPass aRenderPass)
{
    VkPipelineViewportStateCreateInfo ViewportState = {};
	ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	ViewportState.pNext = nullptr;

	ViewportState.viewportCount = 1;
	ViewportState.pViewports = &m_Viewport;
	ViewportState.scissorCount = 1;
	ViewportState.pScissors = &m_Scissor;

	VkPipelineColorBlendStateCreateInfo ColorBlending = {};
	ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	ColorBlending.pNext = nullptr;

	ColorBlending.logicOpEnable = VK_FALSE;
	ColorBlending.logicOp = VK_LOGIC_OP_COPY;
	ColorBlending.attachmentCount = static_cast<uint32_t>(m_ColorBlendAttachment.size());
	ColorBlending.pAttachments = m_ColorBlendAttachment.data();


	VkGraphicsPipelineCreateInfo PipelineInfo = {};
	PipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	PipelineInfo.pNext = nullptr;

	PipelineInfo.stageCount = static_cast<uint32_t>(m_ShaderStages.size());
	PipelineInfo.pStages = m_ShaderStages.data();
	PipelineInfo.pVertexInputState = &m_VertexInputInfo;
	PipelineInfo.pInputAssemblyState = &m_InputAssembly;
	PipelineInfo.pViewportState = &ViewportState;
	PipelineInfo.pDepthStencilState = &m_DepthStencil;
	PipelineInfo.pRasterizationState = &m_Rasterizer;
	PipelineInfo.pMultisampleState = &m_Multisampling;
	PipelineInfo.pColorBlendState = &ColorBlending;
    PipelineInfo.pDynamicState = &m_DynamicState;
	PipelineInfo.layout = m_PipelineLayout;
	PipelineInfo.renderPass = aRenderPass;
	PipelineInfo.subpass = 0;
	PipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline NewPipeline;
	if (vkCreateGraphicsPipelines(
		aDevice, VK_NULL_HANDLE, 1, &PipelineInfo, nullptr, &NewPipeline) != VK_SUCCESS)
	{
		std::cout << "failed to create pipeline\n";
		return VK_NULL_HANDLE;
	}
	else
	{
		return NewPipeline;
	}
}
