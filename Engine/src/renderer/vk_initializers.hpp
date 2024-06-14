#pragma once

#include <core/defines.h>
#include "vk_types.hpp"
namespace vkinit
{
    VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32 aQueueFamilyIndex, VkCommandPoolCreateFlags aFlags = 0);

    VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool aCommandPool, uint32 aCount = 1, VkCommandBufferLevel aLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}