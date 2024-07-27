#pragma once

#include <glm/mat4x4.hpp>

#include <string>
#include <vector>
#include <memory>

/**
 * @brief Struct that holds the base and immutable rendering info for an object, 
 * including its mesh and material.
 */
struct sRenderObjectInfo
{
    sRenderObjectInfo(){}
    sRenderObjectInfo(const std::string& aMeshPath, const std::string& aTexturePath);

    // TODO: Store everything with IDs instead of strings.
    std::string MeshPath;
    std::string TexturePath;
    std::string MaterialName;

    bool operator==(const sRenderObjectInfo& aOther) const 
    {
        return MeshPath == aOther.MeshPath && TexturePath == aOther.TexturePath;
    }
};

/**
 * @brief Struct that holds all the information needed for rendering an object.
 * It holds a sRenderObjectInfo for all the immutable state. The rest of the variables
 * hold the mutable state.
 */
struct sRenderObject
{
    glm::mat4 ModelMatrix;
    sRenderObjectInfo* pRenderObjectInfo;
};

/**
 * @brief Class that represents a rendering scene. It includes all the objects to be renderer and will
 * include all lighting configuration.
 */
class CScene
{
public:
    CScene();
    ~CScene();

    void AddRenderObject(sRenderObject* const apRenderObject);

    const std::vector<sRenderObject*>& GetRenderObjects() { return m_RenderObjects; }

private:
    std::vector<sRenderObject*> m_RenderObjects;
};