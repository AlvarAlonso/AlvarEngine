#pragma once

#include <core/defines.h>
#include "vk_types.hpp"
#include <vector>

namespace vkinit
{
    VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32 aQueueFamilyIndex, VkCommandPoolCreateFlags aFlags = 0);

    VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool aCommandPool, uint32 aCount = 1, VkCommandBufferLevel aLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo();

	VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo();

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo(VkPrimitiveTopology aTopology);

    VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo(bool abDepthTest, bool abDepthWrite, VkCompareOp aCompareOp);

	VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo(VkPolygonMode aPolygonMode);
	
    VkPipelineMultisampleStateCreateInfo MultisamplingStateCreateInfo();

	VkPipelineColorBlendAttachmentState ColorBlendAttachmentState();
   	
    VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits aStage, VkShaderModule aShaderModule);
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

	VkPipeline BuildPipeline(VkDevice aDevice, VkRenderPass aRenderPass);
};