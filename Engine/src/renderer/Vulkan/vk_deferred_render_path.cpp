#include "vk_deferred_render_path.hpp"
#include "vulkan_backend.hpp"
#include "vulkan_device.hpp"
#include "vulkan_swapchain.hpp"
#include "vk_initializers.hpp"
#include "vk_utils.hpp"

#include <iostream>
#include <array>
    
CVulkanDeferredRenderPath::CVulkanDeferredRenderPath(CVulkanBackend* apVulkanBackend, CVulkanDevice* apVulkanDevice, CVulkanSwapchain* apVulkanSwapchain) :
    m_pVulkanBackend(apVulkanBackend), m_pVulkanDevice(apVulkanDevice), m_pVulkanSwapchain(apVulkanSwapchain)
{
}

void CVulkanDeferredRenderPath::CreateResources()
{
    CreateDeferredAttachments();
    CreateGBufferDescriptors();
    CreateDeferredRenderPass();
    CreateDeferredPipeline();
}
    
void CVulkanDeferredRenderPath::DestroyResources()
{
    m_MainDeletionQueue.Flush();
}

void CVulkanDeferredRenderPath::RecordCommands(VkCommandBuffer aCommandBuffer, uint32_t aImageIdx)
{

}

void CVulkanDeferredRenderPath::Render(const CCamera* const aCamera)
{

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

	vkCreateDescriptorPool(m_pVulkanDevice->m_Device, &PoolInfo, nullptr, &m_GBufferPool);

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
	DeferredSetAlloc.descriptorPool = m_GBufferPool;
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

void CVulkanDeferredRenderPath::CreateDeferredPipeline()
{
	VkShaderModule DeferredVertShader;
	// TODO: Do not hardcode this.
	if (!vkutils::LoadShaderModule(m_pVulkanDevice->m_Device, "../Engine/shaders/vert.spv", &DeferredVertShader))
	{
		std::cout << "Error when building the deferred vertex shader module" << std::endl;
	}
	else
	{
		std::cout << "Deferred Vertex Shader loaded SUCCESSFULLY!" << std::endl;
	}

	VkShaderModule DeferredFragShader;
	if (!vkutils::LoadShaderModule(m_pVulkanDevice->m_Device, "../Engine/shaders/frag.spv", &DeferredFragShader))
	{
		std::cout << "Error when building the deferred fragment shader module" << std::endl;
	}
	else
	{
		std::cout << "Deferred Fragment Shader loaded SUCCESSFULLY!" << std::endl;
	}

	VkShaderModule LightVertexShader;
	if (!vkutils::LoadShaderModule(m_pVulkanDevice->m_Device, "../Engine/shaders/vert.spv", &LightVertexShader))
	{
		std::cout << "Error when building the light vertex shader module" << std::endl;
	}
	else
	{
		std::cout << "Light Vertex Shader loaded SUCCESSFULLY!" << std::endl;
	}

	VkShaderModule LightFragShader;
	if (!vkutils::LoadShaderModule(m_pVulkanDevice->m_Device, "../Engine/shaders/frag.spv", &LightFragShader))
	{
		std::cout << "Error when building the light fragment shader module" << std::endl;
	}
	else
	{
		std::cout << "Light Fragment Shader loaded SUCCESSFULLY!" << std::endl;
	}

	// Layouts
	VkPipelineLayoutCreateInfo LayoutInfo = vkinit::PipelineLayoutCreateInfo();

	std::array<VkDescriptorSetLayout, 3> deferredSetLayouts = { m_pVulkanBackend->m_DescriptorSetLayout, m_pVulkanBackend->m_RenderObjectsSetLayout, m_SingleTextureSetLayout };

	LayoutInfo.pushConstantRangeCount = 0;
	LayoutInfo.pPushConstantRanges = nullptr;
	LayoutInfo.setLayoutCount = static_cast<uint32_t>(deferredSetLayouts.size());
	LayoutInfo.pSetLayouts = deferredSetLayouts.data();

	VK_CHECK(vkCreatePipelineLayout(m_pVulkanDevice->m_Device, &LayoutInfo, nullptr, &m_DeferredPipelineLayout));

	std::array<VkDescriptorSetLayout, 2> lightSetLayouts = { m_pVulkanBackend->m_DescriptorSetLayout, m_GBufferSetLayout };

	LayoutInfo.setLayoutCount = static_cast<uint32_t>(lightSetLayouts.size());
	LayoutInfo.pSetLayouts = lightSetLayouts.data();

	VK_CHECK(vkCreatePipelineLayout(m_pVulkanDevice->m_Device, &LayoutInfo, nullptr, &m_LightPipelineLayout));


	// G-Buffers Pipeline Creation.
	PipelineBuilder PipelineBuilder;

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

	m_LightPipeline = PipelineBuilder.BuildPipeline(m_pVulkanDevice->m_Device, m_DeferredRenderPass);

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
