#pragma once

#include <engine.hpp>
#include <core/logger.h>

#include <string>
#include <unordered_map>

class CTexture
{
public:
    static std::unordered_map<std::string, CTexture*> m_LoadedTextures; 

    template<class T>
    static T* Get(const std::string& aFilePath)
    {
        // Check if we already have the file stored in m_LoadedTextures and is of the requested type.
        const auto& FoundFile = m_LoadedTextures.find(aFilePath);
        if (FoundFile != m_LoadedTextures.cend())
        {
            T* Texture = dynamic_cast<T*>(FoundFile->second);
            if (Texture == nullptr)
            {
                SGSERROR("Texture casted to wrong type!");
            }

            return Texture;
        }

        T* pCreatedTexture = CTexture::Create<T>(std::move(aFilePath));
        pCreatedTexture->m_Filename = aFilePath;
        if (pCreatedTexture)
        {
            m_LoadedTextures.insert({ aFilePath, pCreatedTexture});
        }

        return pCreatedTexture;
    }

    static CTexture* Create(const uint64_t aImageSize, void *aPixel_Ptr, int32_t aTexWidth, int32_t aTexHeight);
    static void RegisterTexture(CTexture* apTexture);

    CTexture() = default;

    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;

    void SetID(const std::string& aID) { m_ID = aID; }
    const std::string& GetFilename() const { return m_Filename; }

protected:
    std::string m_ID;
    std::string m_Filename;

private:
    template<class T>
    static T* Create(const std::string& aFilePath)
    {
        return dynamic_cast<T*>(CreateImpl(aFilePath));
    }

    static CTexture* CreateImpl(const std::string& aFilePath);
};
