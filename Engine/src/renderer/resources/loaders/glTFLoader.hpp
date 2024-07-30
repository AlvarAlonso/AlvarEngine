#pragma once

#include <string>
#include <vector>

#include <glm/mat4x4.hpp>

struct sMeshData;

// TODO: Move this.
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