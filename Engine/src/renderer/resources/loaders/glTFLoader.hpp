#pragma once

#include <string>
#include <vector>

#include <glm/mat4x4.hpp>

struct sMeshData;
class CMaterial;

// TODO: Move this.

/**
 * @brief Used to build hierarchy of meshes, where transform from child meshes are local to parent's transform.
 * Will be the basis for the scene graph. 
 */
class CMeshNode
{
public:
    CMeshNode() = default;
    ~CMeshNode();

    std::string m_Name;
    bool m_bVisible;
    bool m_bOpaque;
    glm::mat4 m_Model;

    sMeshData* m_pMeshData;
    CMeshNode* m_pParent;
    std::vector<CMeshNode*> m_Children;
};

/**
 * @brief Represents a submesh from a complete mesh. A mesh can have multiple primitives/submeshes with different materials. 
 * The CPrimitive has the information of which vertices/indices to use from the CMesh.
 */
class CPrimitive
{
public:
	CPrimitive(uint32_t aFirstVertex, uint32_t aFirstIndex, uint32_t aIndexCount, uint32_t aVertexCount, CMaterial* aMaterial) : m_FirstIndex(aFirstIndex), m_IndexCount(aIndexCount), m_VertexCount(aVertexCount), m_Material(m_Material) 
    {
	};

    uint32_t m_FirstVertex;
    uint32_t m_FirstIndex;
    uint32_t m_IndexCount;
    uint32_t m_VertexCount;
    CMaterial* m_Material;
};

class CRenderable
{
public:
    CRenderable() = default;
    ~CRenderable();

    std::string m_Name;
    CMeshNode* m_pRoot;
};
// ---------------

void LoadGLTF(const std::string& aFilePath, float aScale);