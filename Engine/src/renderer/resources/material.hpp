#pragma once

#include "glm/gtc/matrix_transform.hpp"

#include <string>
#include <unordered_map>

class Texture2D;

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
    Texture2D* pAlbedoTexture;
    Texture2D* pEmissiveTexture;
    Texture2D* pMetallicRoughnessTexture;
    Texture2D* pOcclusionTexture;
    Texture2D* pNormalTexture;
};

class CMaterial
{
public:
    static std::unordered_map<std::string, CMaterial*> m_LoadedMaterials;
    static CMaterial* Get(const std::string& aID);

    CMaterial() = default;
    void SetMaterialProperties(const sMaterialProperties& aMaterialProperties);

private:
    std::string m_ID;
    sMaterialProperties m_MaterialProperties;
};