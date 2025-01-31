#pragma once

#include <string>

namespace Alvar
{
    class CRenderable;

    CRenderable* LoadGLTF(const std::string& aFilePath, float aScale);
}
