#pragma once

class CCamera;

/**
 * @brief Interface to interact a RenderPath and manage its resources and commands.
 */
class IRenderPath
{
public:
    virtual void CreateResources() = 0;
    virtual void DestroyResources() = 0;
    virtual void Render(const CCamera* const aCamera) = 0;
};