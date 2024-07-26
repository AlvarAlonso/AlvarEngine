#include "texture.hpp"
#include <renderer/vulkan/resources/vk_texture.hpp>
#include <renderer/render_module.hpp>
#include <renderer/core/render_types.hpp>

std::unordered_map<std::string, CTexture*> CTexture::m_LoadedTextures; 

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
            return new CVkTexture();
        }
        break;
    
    default:
        SGSERROR("No graphics API selected!");
        break;
    }

    return nullptr;
}
