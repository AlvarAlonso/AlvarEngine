#pragma once

#include <core/defines.h>
#include "vk_types.hpp"

namespace vkinit
{
    VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32 aQueueFamilyIndex, VkCommandPoolCreateFlags aFlags = 0);

    VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool aCommandPool, uint32 aCount = 1, VkCommandBufferLevel aLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkDescriptorSetLayoutBinding DescriptorLayoutBinding(VkDescriptorType aType, VkShaderStageFlags aStageFlags, uint32_t aBinding);
	
	VkWriteDescriptorSet WriteDescriptorBuffer(VkDescriptorType aType, VkDescriptorSet aDstSet, VkDescriptorBufferInfo* aBufferInfo, uint32_t aBinding, uint32_t aCount = 1);

	VkWriteDescriptorSet WriteDescriptorImage(VkDescriptorType aType, VkDescriptorSet aDstSet, VkDescriptorImageInfo* aImageInfo, uint32_t aBinding);

	VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo();

    VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo(const std::vector<VkDynamicState>& aDynamicStates);

	VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo();

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo(VkPrimitiveTopology aTopology);

    VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo(bool abDepthTest, bool abDepthWrite, VkCompareOp aCompareOp);

	VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo(VkPolygonMode aPolygonMode);
	
    VkPipelineMultisampleStateCreateInfo MultisamplingStateCreateInfo();

	VkPipelineColorBlendAttachmentState ColorBlendAttachmentState();

    VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits aStage, VkShaderModule aShaderModule);

	VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags aFlags = 0);

    VkRenderPassBeginInfo RenderPassBeginInfo(VkRenderPass aRenderPass, VkExtent2D aWindowExtent, VkFramebuffer aFramebuffer);

	VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags aFlags = 0);

	VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags aFlags = 0);

    VkSubmitInfo SubmitInfo(VkCommandBuffer* aCmd);

	VkPresentInfoKHR PresentInfo();

	VkImageCreateInfo ImageCreateInfo(VkFormat aFormat, VkImageUsageFlags aUsageFlags, VkExtent3D aExtent);

	VkImageViewCreateInfo ImageViewCreateInfo(VkFormat aFormat, VkImage aImage, VkImageAspectFlags aAspectFlags);
}

// TODO: Make namespaces for everything related to my code.
class PipelineBuilder {
public:
	std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
	VkPipelineVertexInputStateCreateInfo m_VertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo m_InputAssembly;
	VkViewport m_Viewport;
	VkRect2D m_Scissor;
	VkPipelineDepthStencilStateCreateInfo m_DepthStencil;
	VkPipelineRasterizationStateCreateInfo m_Rasterizer;
	std::vector<VkPipelineColorBlendAttachmentState> m_ColorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo m_Multisampling;
	VkPipelineLayout m_PipelineLayout;
    VkPipelineDynamicStateCreateInfo m_DynamicState;

	VkPipeline BuildPipeline(VkDevice aDevice, VkRenderPass aRenderPass);
};