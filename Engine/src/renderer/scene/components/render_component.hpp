#pragma once

#include <renderer/core/render_types.hpp>
#include <vector>

namespace Alvar
{
    struct sRenderComponent
    {
        sRenderComponent();
    
        sMeshData* MeshData;
        std::vector<CMaterial*> Materials;   // One material for each submesh.
        bool IsVisible;
    };
}
