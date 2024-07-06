#include "scene.hpp"

sRenderObjectInfo::sRenderObjectInfo(const std::string& aMeshPath, const std::string& aTexturePath) :
    MeshPath(std::move(aMeshPath)), TexturePath(std::move(aTexturePath))
{
}

CScene::CScene() :
    m_RenderObjects()
{
}

CScene::~CScene()
{
    for (auto& RenderObject : m_RenderObjects)
    {
        delete RenderObject;
    }
}

void CScene::AddRenderObject(sRenderObject* const apRenderObject)
{
    m_RenderObjects.emplace_back(apRenderObject);
}
