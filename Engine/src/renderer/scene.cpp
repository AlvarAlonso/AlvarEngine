#include "scene.hpp"

sRenderObjectInfo::sRenderObjectInfo(const std::string& aMeshPath, const std::string& aTexturePath) :
    MeshPath(std::move(aMeshPath)), TexturePath(std::move(aTexturePath))
{
}

CScene::CScene() :
    m_Renderables()
{
}

CScene::~CScene()
{
    for (auto& Renderable : m_Renderables)
    {
        delete Renderable;
    }
}

void CScene::AddRenderable(CRenderable* const apRenderable)
{
    m_Renderables.emplace_back(apRenderable);
}
