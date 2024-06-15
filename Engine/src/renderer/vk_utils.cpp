#include "vk_utils.hpp"

#include <iostream>
#include <fstream>
#include <vector>

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
