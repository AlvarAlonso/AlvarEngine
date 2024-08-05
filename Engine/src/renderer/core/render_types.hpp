#pragma once

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <string>
#include <vector>
#include <unordered_map>

// TODO: Change that.
#include <renderer/resources/loaders/glTFLoader.hpp>

class CMaterial;

struct sVertex
{
    sVertex();

    sVertex(const glm::vec3& aPosition, const glm::vec3& aNormal, const glm::vec3& aColor, const glm::vec2& aUV);

    glm::vec3 Position;
    glm::vec3 Normal;
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
				(hash<glm::vec3>()(Vertex.Normal) << 1)) >> 1) ^
				(hash<glm::vec2>()(Vertex.UV) << 1);
		}
	};
}

/**
 * @brief Represents a submesh from a complete mesh. A mesh can have multiple primitives/submeshes with different materials. 
 * The CPrimitive has the information of which vertices/indices to use from the CMesh.
 */
class CSubMesh
{
public:
	CSubMesh(uint32_t aFirstVertex, uint32_t aFirstIndex, uint32_t aIndexCount, uint32_t aVertexCount, CMaterial* aMaterial) : 
        m_FirstVertex(aFirstVertex), m_FirstIndex(aFirstIndex), m_IndexCount(aIndexCount), m_VertexCount(aVertexCount), m_Material(aMaterial) 
    {
	};

    uint64_t m_FirstVertex;
    uint64_t m_FirstIndex;
    uint64_t m_IndexCount;
    uint64_t m_VertexCount;
    CMaterial* m_Material;
};

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

    std::vector<CSubMesh*> SubMeshes;

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

enum class eRenderPath : uint8_t
{
    FORWARD = 0,
    DEFERRED,
    NUM
};

enum class eRenderAPI : uint8_t
{
    NONE = 0,
    VULKAN,
    NUM
};

/**
 * @brief Used to build hierarchy of meshes, where transform from child meshes are local to parent's transform.
 * Will be the basis for the scene graph. 
 */
class CMeshNode
{
public:
    CMeshNode();
    ~CMeshNode();

    glm::mat4 GetWorldTransform(bool bFast = false);

    std::string m_Name;
    bool m_bVisible;
    bool m_bOpaque;
    glm::mat4 m_Model;
    glm::mat4 m_WorldModel;

    sMeshData* m_pMeshData;
    CMeshNode* m_pParent;
    std::vector<CMeshNode*> m_Children;
};

class CRenderable
{
public:
    static CRenderable* Create();
    /**
     * @brief Creates a CRenderable with a node containing the info passed as sMeshData.
     * @param apMeshData The apMeshData to create the node from.
     * @return The new renderable.
     */
    static CRenderable* Create(sMeshData* apMeshData);

    CRenderable() = default;
    CRenderable(sMeshData* apMeshData);
    ~CRenderable();

    virtual void UploadToVRAM() = 0;

    std::string m_Name;
    std::vector<CMeshNode*> m_pRoots;
    std::vector<sVertex> m_Vertices;
    std::vector<uint32_t> m_Indices;
    uint32_t m_VerticesCount;
    uint32_t m_IndicesCount;
};