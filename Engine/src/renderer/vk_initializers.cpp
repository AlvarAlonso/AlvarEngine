#include "vk_initializers.hpp"

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
    CreateInfo.level = aLevel;

    return CreateInfo;
}
