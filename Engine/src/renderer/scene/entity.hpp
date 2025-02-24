#pragma once

#include <core/assertions.h>
#include <entt/src/entt/entt.hpp> // TODO: wtf.

namespace Alvar
{
    class CScene;

    class CEntity
    {
    public:
        CEntity() = default;

        template<typename T, typename... Args>
        T& AddComponent(Args&&... aArgs)
        {
            SGSASSERT_MSG(!HasComponent<T>(), "Already has the component!");
            T& Component = m_pScene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(aArgs)...);
        }

        template<typename T>
        T& Getcomponent()
        {
            return m_pScene->m_Registry.get<T>(m_EntityHandle);
        }

        template<typename T>
        bool HasComponent()
        {
            return m_pScene->m_Registry.any_of<T>(m_EntityHandle);
        }

        template<typename T>
        bool RemoveComponent()
        {
            m_pScene->m_Registry.remove<T>(m_EntityHandle);
        }

        operator bool() const { return m_EntityHandle != entt::null; }
    
    private:
        entt::entity m_EntityHandle { entt::null };
        CScene* m_pScene = nullptr;
    };
}