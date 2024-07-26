#include "vk_texture.hpp"
#include <renderer/vulkan/vk_utils.hpp>
#include <renderer/vulkan/vulkan_device.hpp>

CVkTexture::CVkTexture(const std::string& aFilePath)
{
    vkutils::LoadImageFromFile(GetVulkanDevice(), aFilePath, m_AllocatedImage);
}
