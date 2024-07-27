#pragma once

#include <renderer/vulkan/vk_types.hpp>
#include <renderer/resources/texture.hpp>

#include <string>

class CVkTexture : public CTexture
{
public:
    CVkTexture() = default;
    CVkTexture(const std::string& aFilePath);

    virtual uint32_t GetWidth() const override { return m_Width; }
    virtual uint32_t GetHeight() const override { return m_Height; }

    AllocatedImage GetAllocatedImage() const { return m_AllocatedImage; }
    VkImageView GetImageView() const { return m_ImageView; }

private:
    uint32_t m_Width;
    uint32_t m_Height;
    AllocatedImage m_AllocatedImage;
    VkImageView m_ImageView;
};
