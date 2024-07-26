#include "material.hpp"

#include <core/logger.h>

std::unordered_map<std::string, CMaterial*> CMaterial::m_LoadedMaterials;

CMaterial* CMaterial::Get(const std::string& aID)
{
    // Check if we already have the file stored in m_LoadedMaterials.
    const auto& FoundMaterial = m_LoadedMaterials.find(aID);
    if (FoundMaterial != m_LoadedMaterials.cend())
    {
        return FoundMaterial->second;
    }
    else
    {
        SGSWARN("The material %s does not exist.", aID.c_str());
        return nullptr;
    }
}

void CMaterial::SetMaterialProperties(const sMaterialProperties& aMaterialProperties)
{
    m_MaterialProperties = aMaterialProperties;
}
