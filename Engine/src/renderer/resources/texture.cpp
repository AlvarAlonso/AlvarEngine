#include "texture.hpp"
#include <renderer/vulkan/resources/vk_texture.hpp>
#include <renderer/render_module.hpp>
#include <renderer/core/render_types.hpp>

std::unordered_map<std::string, CTexture*> CTexture::m_LoadedTextures; 

CTexture* CTexture::Create(const uint64_t aImageSize, void *aPixel_Ptr, int32_t aTexWidth, int32_t aTexHeight)
{
    const eRenderAPI RenderAPI = CEngine::Get()->GetRenderModule()->GetRenderAPI();
    switch (RenderAPI)
    {
        case eRenderAPI::NONE:
        {
            SGSERROR("No graphics API selected!");
        }
        break;

        case eRenderAPI::VULKAN:
        {
            return new CVkTexture(aImageSize, aPixel_Ptr, aTexWidth, aTexHeight);
        }
        break;
    
    default:
        SGSERROR("No graphics API selected!");
        break;
    }

    return nullptr;
}

CTexture* CTexture::CreateImpl(const std::string& aFilePath)
{
    const eRenderAPI RenderAPI = CEngine::Get()->GetRenderModule()->GetRenderAPI();
    switch (RenderAPI)
    {
        case eRenderAPI::NONE:
        {
            SGSERROR("No graphics API selected!");
        }
        break;

        case eRenderAPI::VULKAN:
        {
            return new CVkTexture(aFilePath);
        }
        break;
    
    default:
        SGSERROR("No graphics API selected!");
        break;
    }

    return nullptr;
}

void CTexture::RegisterTexture(CTexture* apTexture)
{
    const std::string TextureID = apTexture->m_ID;
    if (TextureID.empty())
    {
        SGSERROR("Can't register the texture. The ID is empty!");
    }
    else
    {
        const auto& FoundFile = m_LoadedTextures.find(TextureID);
        if (FoundFile != m_LoadedTextures.cend())
        {
            SGSWARN("Texture with ID: %s is already registered!", TextureID.c_str());
        }
        else
        {
            m_LoadedTextures.insert({ TextureID, apTexture});
        }
    }
}
