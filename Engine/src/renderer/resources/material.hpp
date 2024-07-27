#pragma once

#include "glm/gtc/matrix_transform.hpp"

#include <string>
#include <unordered_map>

class CTexture;

struct sMaterialProperties
{
    bool bIsTransparent;

    // Property factors.
    glm::vec4 Color;
    float RoughnessFactor;
    float MetallicFactor;
    float TillingFactor;
    glm::vec3 EmissiveFactor;

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
    static void Add(const std::string& aID, CMaterial* aMaterial);

    CMaterial() = default;
    void SetMaterialProperties(const sMaterialProperties& aMaterialProperties);
    sMaterialProperties GetMaterialProperties() const { return m_MaterialProperties; }

private:
    static std::unordered_map<std::string, CMaterial*> m_LoadedMaterials;

    std::string m_ID;
    sMaterialProperties m_MaterialProperties;
};