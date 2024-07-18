#include "vk_types.hpp"
#include <core/logger.h>

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

	VkVertexInputAttributeDescription ColorAttribute = {};
	ColorAttribute.binding = 0;
	ColorAttribute.location = 1;
	ColorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	ColorAttribute.offset = offsetof(sVertex, Color);

	VkVertexInputAttributeDescription UVAttribute = {};
	UVAttribute.binding = 0;
	UVAttribute.location = 2;
	UVAttribute.format = VK_FORMAT_R32G32_SFLOAT;
	UVAttribute.offset = offsetof(sVertex, UV);

	Description.Attributes.push_back(PositionAttribute);
	Description.Attributes.push_back(ColorAttribute);
	Description.Attributes.push_back(UVAttribute);

	return Description;
}

std::unordered_map<std::string, sMesh*> sMesh::LoadedMeshes;

sMesh* sMesh::GetMesh(const std::string& aID)
{
    const auto& FoundMesh = LoadedMeshes.find(aID);
    if (FoundMesh != LoadedMeshes.cend())
    {
        return FoundMesh->second;
    }
    else 
    {
		SGSERROR("Vulkan mesh with ID %s does not exist!", aID.c_str());
		return nullptr;
    }
}

bool sMesh::HasMesh(const std::string& aID)
{
	const auto& FoundMesh = LoadedMeshes.find(aID);
	return FoundMesh != LoadedMeshes.cend();
}

sMesh::sMesh(const std::string& aID) : ID(std::move(aID))
{
}
