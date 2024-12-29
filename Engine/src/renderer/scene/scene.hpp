#pragma once

#include <renderer/core/render_types.hpp>
#include <renderer/scene/entities/light_source.hpp>

#include <glm/mat4x4.hpp>

#include <string>
#include <vector>
#include <memory>

/**
 * @brief Class that represents a rendering scene. It includes all the objects to be renderer and will
 * include all lighting configuration.
 */
class CScene
{
public:
    CScene();
    ~CScene();

    void AddRenderable(CRenderable* const apRenderable);
    void AddLightSource(sLightSource* const apLightSource);

    const std::vector<CRenderable*>& GetRenderables() const { return m_Renderables; }
    const std::vector<sLightSource*>& GetLightSources() const { return m_LightSources; }

private:
    std::vector<CRenderable*> m_Renderables;
    std::vector<sLightSource*> m_LightSources;
};