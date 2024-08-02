#include "vk_texture.hpp"
#include <renderer/vulkan/vk_utils.hpp>
#include <renderer/vulkan/vulkan_device.hpp>
#include <renderer/vulkan/vk_initializers.hpp>

CVkTexture::CVkTexture(const std::string& aFilePath)
{
    vkutils::LoadImageFromFile(GetVulkanDevice(), aFilePath, m_AllocatedImage);

    VkImageViewCreateInfo ViewInfo = vkinit::ImageViewCreateInfo(VK_FORMAT_R8G8B8A8_SRGB, m_AllocatedImage.Image, VK_IMAGE_ASPECT_COLOR_BIT);
   
    CVulkanDevice* pDevice = GetVulkanDevice();
    VK_CHECK(vkCreateImageView(pDevice->m_Device, &ViewInfo, nullptr, &m_ImageView));

    // TODO: To be removed. Free resources in another place.
    pDevice->m_MainDeletionQueue.PushFunction([=]
    {
        vkDestroyImageView(pDevice->m_Device, m_ImageView, nullptr);
        vmaDestroyImage(pDevice->m_Allocator, m_AllocatedImage.Image, m_AllocatedImage.Allocation);
    });
}

CVkTexture::CVkTexture(const uint64_t aImageSize, void *aPixel_Ptr, int32_t aTexWidth, int32_t aTexHeight)
{
    vkutils::UploadImageToVRAM(GetVulkanDevice(), aImageSize, aPixel_Ptr, aTexWidth, aTexHeight, m_AllocatedImage);
    
    VkImageViewCreateInfo ViewInfo = vkinit::ImageViewCreateInfo(VK_FORMAT_R8G8B8A8_SRGB, m_AllocatedImage.Image, VK_IMAGE_ASPECT_COLOR_BIT);

    CVulkanDevice* pDevice = GetVulkanDevice();
    VK_CHECK(vkCreateImageView(pDevice->m_Device, &ViewInfo, nullptr, &m_ImageView));
    
    // TODO: To be removed. Free resources in another place.
    pDevice->m_MainDeletionQueue.PushFunction([=]
    {
        vkDestroyImageView(pDevice->m_Device, m_ImageView, nullptr);
        vmaDestroyImage(pDevice->m_Allocator, m_AllocatedImage.Image, m_AllocatedImage.Allocation);
    });
}
