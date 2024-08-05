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

void CMaterial::RegisterMaterial(CMaterial* apMaterial)
{
    const std::string MaterialID = apMaterial->m_ID;
    if (MaterialID.empty())
    {
        SGSERROR("Can't register the material. The ID is empty!");
    }
    else
    {
        const auto& FoundMaterial = m_LoadedMaterials.find(MaterialID);
        if (FoundMaterial != m_LoadedMaterials.cend())
        {
            SGSWARN("The material with ID: %s is already registered!", MaterialID.c_str());
        }
        else
        {
            m_LoadedMaterials.insert({MaterialID, apMaterial});
        }
    }
}
    
void CMaterial::SetID(const std::string& aID)
{
    if (!m_ID.empty())
    {
        auto NodeHandler = m_LoadedMaterials.extract(m_ID);
        if (!NodeHandler.empty())
        {
            NodeHandler.key() = aID;
            m_LoadedMaterials.insert(std::move(NodeHandler));
        }
        else
        {
            m_LoadedMaterials.insert({aID, this});
        }
    }

    m_ID = aID;
}

void CMaterial::SetMaterialProperties(const sMaterialProperties& aMaterialProperties)
{
    m_MaterialProperties = aMaterialProperties;
}
