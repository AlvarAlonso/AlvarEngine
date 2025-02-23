#include "scene.hpp"
#include "entity.hpp"

namespace Alvar
{
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

    void CScene::AddLightSource(sLightSource* const apLightSource)
    {
        m_LightSources.emplace_back(apLightSource);
    }
}
