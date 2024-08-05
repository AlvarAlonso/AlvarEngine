#pragma once

#include "glm/gtc/matrix_transform.hpp"

#include <string>
#include <unordered_map>

class CTexture;

struct sMaterialConstants
{
    glm::vec4 Color;
    float RoughnessFactor;
    float MetallicFactor;
    float TillingFactor;
    glm::vec3 EmissiveFactor;
    bool bIsTransparent;
};
 
struct sMaterialProperties
{
    sMaterialConstants MaterialConstants;

    // Textures.
    CTexture* pAlbedoTexture;
    CTexture* pEmissiveTexture;
    CTexture* pMetallicRoughnessTexture;
    CTexture* pOcclusionTexture;
    CTexture* pNormalTexture;
};

class CMaterial
{
public:
    static CMaterial* Get(const std::string& aID);
    static void RegisterMaterial(CMaterial* apMaterial);

    CMaterial() = default;
    void SetID(const std::string& aID);
    std::string GetID() const { return m_ID; }
    void SetMaterialProperties(const sMaterialProperties& aMaterialProperties);
    sMaterialProperties GetMaterialProperties() const { return m_MaterialProperties; }
    sMaterialConstants GetMaterialConstatns() const { return m_MaterialProperties.MaterialConstants; }

private:
    static std::unordered_map<std::string, CMaterial*> m_LoadedMaterials;

    std::string m_ID;
    sMaterialProperties m_MaterialProperties;
};