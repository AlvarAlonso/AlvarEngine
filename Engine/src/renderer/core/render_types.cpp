#include "render_types.hpp"
#include "render_utils.hpp"
#include "geometry_generator.hpp"
#include <core/logger.h>

#include <unordered_map>

enum class eGEOMETRY_PRIMITIVE
{
    BOX = 0,
    SPHERE,
    GRID,
    QUAD,
    NUM
};

// TODO: xd.
static std::unordered_map<std::string, eGEOMETRY_PRIMITIVE> GeometryPrimitiveNames = 
    {{"box", eGEOMETRY_PRIMITIVE::BOX}, 
    {"sphere", eGEOMETRY_PRIMITIVE::SPHERE}, 
    {"grid", eGEOMETRY_PRIMITIVE::QUAD}, 
    {"quad", eGEOMETRY_PRIMITIVE::QUAD} };

sVertex::sVertex() :
    Position(0.0f, 0.0f, 0.0f), Color(1.0f, 1.0f, 1.0f), UV(0.0f, 0.0f)
{
}

sVertex::sVertex(const glm::vec3& aPosition, const glm::vec3& aColor, const glm::vec2& aUV) :
    Position(std::move(aPosition)), Color(std::move(aColor)), UV(std::move(aUV))
{
}

sMeshData::sMeshData() : ID(""), Vertices(), Indices32(), Indices16()
{
}

sMeshData::sMeshData(const sMeshData& aMeshData)
{
    ID = std::move(aMeshData.ID);
    Vertices = std::move(aMeshData.Vertices);
    Indices32 = std::move(aMeshData.Indices32);
    Indices16 = std::move(aMeshData.Indices16);
}

std::unordered_map<std::string, sMeshData*> sMeshData::LoadedMeshes;

// TODO: Better way to differentiate between procedural and path meshes.
// In the final engine, all meshes should have an ID and the engine should not care about the real path, 
// since it should work with the IDs.
sMeshData* sMeshData::GetMeshData(const std::string& aFilename)
{
    bool bIsPrimitiveID = GeometryPrimitiveNames.find(aFilename) != GeometryPrimitiveNames.cend();
    return bIsPrimitiveID ? GetMeshDataFromProceduralPrimitive(aFilename) : GetMeshDataFromFile(aFilename);
}

sMeshData* sMeshData::GetMeshDataFromFile(const std::string& aFilename)
{
    const auto& FoundMesh = LoadedMeshes.find(aFilename);
    if (FoundMesh != LoadedMeshes.cend())
    {
        return FoundMesh->second;
    }
    else 
    {
        sMeshData* MeshData = new sMeshData();
        if (renderutils::LoadMeshFromFile(aFilename, *MeshData))
        {
            LoadedMeshes[aFilename] = MeshData;
            return LoadedMeshes.at(aFilename);
        }
        else
        {
            delete MeshData;
            return nullptr;
        }
    }
}

// TODO: Right now the params are hardcoded.
sMeshData* sMeshData::GetMeshDataFromProceduralPrimitive(const std::string& aID)
{
    eGEOMETRY_PRIMITIVE Primitive = GeometryPrimitiveNames[aID];

    sMeshData* MeshData = nullptr;

    switch (Primitive)
    {
        case eGEOMETRY_PRIMITIVE::BOX:
        {
            MeshData = new sMeshData(CGeometryGenerator::CreateBox(1.0f, 1.0f, 1.0f, 1.0f));
        }
        break;

        case eGEOMETRY_PRIMITIVE::SPHERE:
        {
            MeshData = new sMeshData(CGeometryGenerator::CreateSphere(1.0f, 12.0f, 12.0f));
        }
        break;

        case eGEOMETRY_PRIMITIVE::GRID:
        {
            MeshData = new sMeshData(CGeometryGenerator::CreateGrid(10.0f, 10.0f, 2.0f, 2.0f));
        }
        break;

        case eGEOMETRY_PRIMITIVE::QUAD:
        {
            MeshData = new sMeshData(CGeometryGenerator::CreateQuad(0.0f, 1.0f, 0.0f, 1.0f, 1.0f));
        }
        break;

        default:
        {
            SGSERROR("Invalid primitive ID passed!");
        }
    }

    return MeshData;
}

bool sMeshData::HasMeshData(const std::string& aFilename)
{
    const auto& FoundMeshData = LoadedMeshes.find(aFilename);
	return FoundMeshData != LoadedMeshes.cend();
}
