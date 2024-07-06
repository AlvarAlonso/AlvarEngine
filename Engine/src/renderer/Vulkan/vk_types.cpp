#include "vk_types.hpp"
#include "vk_utils.hpp"

std::unordered_map<std::string, sMesh*> sMesh::LoadedMeshes;

sMesh* sMesh::GetMesh(const std::string& aFilename)
{
    const auto& FoundMesh = LoadedMeshes.find(aFilename);
    if (FoundMesh != LoadedMeshes.cend())
    {
        return FoundMesh->second;
    }
    else 
    {
        sMesh* Mesh = new sMesh;
        if (vkutils::LoadMeshFromFile(aFilename, *Mesh))
        {
            LoadedMeshes[aFilename] = Mesh;
            return LoadedMeshes.at(aFilename);
        }
        else
        {
            delete Mesh;
            return nullptr;
        }
    }
}
