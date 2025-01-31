#pragma once

#include "render_types.hpp"

#include <string>

namespace Alvar
{
    namespace renderutils
    {
        bool LoadMeshFromFile(const std::string& aFilename, sMeshData& aOutMesh);
    };
}
