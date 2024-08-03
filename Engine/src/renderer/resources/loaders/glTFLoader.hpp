#pragma once

#include <string>

class CRenderable;

CRenderable* LoadGLTF(const std::string& aFilePath, float aScale);