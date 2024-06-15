#pragma once

#include "vk_types.hpp"

namespace vkutils
{
    bool LoadShaderModule(VkDevice aDevice, const char* aFilePath, VkShaderModule* aOutShaderModule);
}