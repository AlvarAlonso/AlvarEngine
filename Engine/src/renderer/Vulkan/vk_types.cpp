#include "vk_types.hpp"
#include "vk_utils.hpp"
#include "vulkan_device.hpp"
#include <core/logger.h>

#include <array>

sVertexInputDescription GetVertexDescription()
{
	sVertexInputDescription Description{};

	VkVertexInputBindingDescription MainBinding = {};
	MainBinding.binding = 0;
	MainBinding.stride = sizeof(sVertex);
	MainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	Description.Bindings.push_back(MainBinding);

	VkVertexInputAttributeDescription PositionAttribute = {};
	PositionAttribute.binding = 0;
	PositionAttribute.location = 0;
	PositionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	PositionAttribute.offset = offsetof(sVertex, Position);

	VkVertexInputAttributeDescription NormalAttribute = {};
    NormalAttribute.binding = 0;
    NormalAttribute.location = 1;
    NormalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    NormalAttribute.offset = offsetof(sVertex, Normal);

	VkVertexInputAttributeDescription ColorAttribute = {};
	ColorAttribute.binding = 0;
	ColorAttribute.location = 2;
	ColorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	ColorAttribute.offset = offsetof(sVertex, Color);

	VkVertexInputAttributeDescription UVAttribute = {};
	UVAttribute.binding = 0;
	UVAttribute.location = 3;
	UVAttribute.format = VK_FORMAT_R32G32_SFLOAT;
	UVAttribute.offset = offsetof(sVertex, UV);

	Description.Attributes.push_back(PositionAttribute);
	Description.Attributes.push_back(NormalAttribute);
	Description.Attributes.push_back(ColorAttribute);
	Description.Attributes.push_back(UVAttribute);

	return Description;
}

CVulkanRenderable::CVulkanRenderable(sMeshData* apMeshData)
{
	m_pRoots.push_back(new CMeshNode());
	m_pRoots[0]->m_pMeshData = apMeshData;
	m_Vertices = apMeshData->Vertices;
	m_Indices = apMeshData->Indices32;
	CSubMesh* pSubMesh = new CSubMesh(0, 0, m_Indices.size(), m_Vertices.size(), nullptr);
	m_pRoots[0]->m_pMeshData->SubMeshes.push_back(pSubMesh);
}
    
void CVulkanRenderable::Draw(sRenderContext& aRenderContext, bool bBindMaterialDescriptor)
{
	// Bind resources once for each renderables. Use offsets to determine which parts are drawn.
	VkDeviceSize Offset = 0;
	vkCmdBindVertexBuffers(aRenderContext.CmdBuffer, 0, 1, &m_VertexBuffer.Buffer, &Offset);
	vkCmdBindIndexBuffer(aRenderContext.CmdBuffer, m_IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

	for (const auto& Root : m_pRoots)
	{
		DrawNode(Root, aRenderContext, bBindMaterialDescriptor);
	}
}
    
void CVulkanRenderable::DrawNode(CMeshNode* apMeshNode, sRenderContext& aRenderContext, bool bBindMaterialDescriptor)
{
	// Draw children.
	for (auto* pMeshNode : apMeshNode->m_Children)
	{
		DrawNode(pMeshNode, aRenderContext, bBindMaterialDescriptor);
	}

	// Draw current node.
	for (const auto& SubMesh : apMeshNode->m_pMeshData->SubMeshes)
	{
		DrawSubMesh(SubMesh, aRenderContext, bBindMaterialDescriptor);
	}
}

void CVulkanRenderable::DrawSubMesh(CSubMesh* apSubMesh, sRenderContext& aRenderContext, bool bBindMaterialDescriptor)
{
	if (bBindMaterialDescriptor)
	{
		const std::array<VkDescriptorSet, 3> DescriptorSets = 
			{ aRenderContext.FrameDescriptorSet, aRenderContext.ObjectsDescriptorSet, aRenderContext.MaterialDescriptors->at(apSubMesh->m_Material->GetID())->DescriptorSet };

		vkCmdBindDescriptorSets(aRenderContext.CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aRenderContext.PipelineLayout, 
			0, static_cast<uint32_t>(DescriptorSets.size()), DescriptorSets.data(), 0, nullptr);
	}

	// TODO: Batch rendering.
	vkCmdDrawIndexed(aRenderContext.CmdBuffer, apSubMesh->m_IndexCount, 1, apSubMesh->m_FirstIndex, apSubMesh->m_FirstVertex, aRenderContext.DrawCallNum);
	++aRenderContext.DrawCallNum;
}

void CVulkanRenderable::UploadToVRAM()
{
	vkutils::CreateVertexBuffer(GetVulkanDevice(), m_Vertices, m_VertexBuffer);
	vkutils::CreateIndexBuffer(GetVulkanDevice(), m_Indices, m_IndexBuffer);

	// TODO: Should this be done in all functions that upload things to GPU?
	// IDEA: Do it like this and just get again from file the vertices/indices in case we detect the buffers are no logner filled and uploaded.
	m_Vertices.clear();
	m_Indices.clear();
}
