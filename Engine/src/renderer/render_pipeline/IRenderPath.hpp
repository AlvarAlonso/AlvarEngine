#pragma once

class CCamera;

/**
 * @brief Interface to interact a RenderPath and manage its resources and commands.
 */
class IRenderPath
{
public:
    /**
     * @brief Creates all resources needed for the IRenderPath.
     */
    virtual void CreateResources() = 0;

    /**
     * @brief Destroy all resources owned by the IRenderPath.
     */
    virtual void DestroyResources() = 0;

    /**
     * @brief Renders the scene.
     * @param aCamera Point of view to render the scene from.
     */
    virtual void Render(const CCamera* const aCamera) = 0;

    /**
     * @brief Updates the content of the buffers accessed by GPU, like uniform buffers.
     */
    virtual void UpdateBuffers() = 0;

    /**
     * @brief Called when the content of the scene to be rendered changes. 
     * Useful to record new commands to adapt to the change.
     */
    virtual void HandleSceneChanged() = 0;
};