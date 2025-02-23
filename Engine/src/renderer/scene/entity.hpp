#pragma once

#include <entt/src/entt/entt.hpp> // TODO: wtf.

namespace Alvar
{
    class CEntity
    {
    public:
        CEntity() = default;
    
    private:
        entt::entity m_EntityHandle { entt::null };
    };
}