#pragma once

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <string>
#include <vector>
#include <unordered_map>

struct sVertex
{
    sVertex();

    sVertex(const glm::vec3& aPosition, const glm::vec3& aColor, const glm::vec2& aUV);

    glm::vec3 Position;
    glm::vec3 Color;
    glm::vec2 UV;

    bool operator==(const sVertex& aOther) const 
    {
        return Position == aOther.Position && UV == aOther.UV;
    }
};

namespace std {
	template<> struct hash<sVertex> {
		size_t operator()(sVertex const& Vertex) const {
			return ((hash<glm::vec3>()(Vertex.Position) ^
				(hash<glm::vec3>()(Vertex.Color) << 1)) >> 1) ^
				(hash<glm::vec2>()(Vertex.UV) << 1);
		}
	};
}

struct sMeshData
{
public:
    sMeshData();
    sMeshData(const sMeshData& aMeshData);
    
    static sMeshData* GetMeshData(const std::string& aFilename);
    static bool HasMeshData(const std::string& aFilename);

    std::string ID;
    std::vector<sVertex> Vertices;
    std::vector<uint32_t> Indices32;

    std::vector<uint16_t>& GetIndices16()
    {
        if (Indices16.empty())
        {
            Indices16.resize(Indices32.size());
            for (size_t i = 0; i < Indices32.size(); ++i)
            {
                Indices16[i] = static_cast<uint16_t>(Indices32[i]);
            }
        }
    }

private:
    static std::unordered_map<std::string, sMeshData*> LoadedMeshes;

    static sMeshData* GetMeshDataFromFile(const std::string& aFilename);
    static sMeshData* GetMeshDataFromProceduralPrimitive(const std::string& aID);

    std::vector<uint16_t> Indices16;
};