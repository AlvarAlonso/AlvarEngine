#include "glTFLoader.hpp"

#define TINYGLTF_IMPLEMENTATION

// TODO: What does this do?
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define STBI_MSC_SECURE_CRT
#include <tinygltf/tiny_gltf.h>

#include <renderer/core/render_types.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

CMeshNode::~CMeshNode()
{
    for (CMeshNode* pMeshNode : m_Children)
    {
        delete pMeshNode;
    }

    delete m_pMeshData;
}

CRenderable::~CRenderable()
{
    delete m_pRoot;
}

static void LoadTextures(tinygltf::Model &aGltfModel)
{

}

static void LoadMaterials(tinygltf::Model &aGltfModel)
{

}

static void LoadNode(CMeshNode* aParent, const tinygltf::Node &aNode, uint32_t aNodeIndex, const tinygltf::Model &aModel, 
    std::vector<uint32_t>& aIndexBuffer, std::vector<sVertex>& aVertexBuffer, float aGlobalScale)
{
    // TODO: find place where to delete this.
    CMeshNode* pNewNode = new CMeshNode();

    // Generate local node matrix.
    glm::vec3 Translation = glm::vec3(0.0f);
    if (aNode.translation.size() == 3)
    {
        Translation = glm::make_vec3(aNode.translation.data());
        pNewNode->m_Model = glm::translate(pNewNode->m_Model, Translation);
    }
    glm::mat4 Rotation = glm::mat4(1.0f);
    if (aNode.rotation.size() == 4)
    {
        glm::quat q = glm::make_quat(aNode.rotation.data());
        pNewNode->m_Model *= glm::mat4(q);
    }
    glm::vec3 Scale = glm::vec3(1.0f);
    if (aNode.scale.size() == 3)
    {
        Scale = glm::make_vec3(aNode.scale.data());
        pNewNode->m_Model = glm::scale(pNewNode->m_Model, Scale);
    }
    if (aNode.matrix.size() == 16)
    {
        pNewNode->m_Model = glm::make_mat4x4(aNode.matrix.data());
    }

    // Node with children.
    if (aNode.children.size() > 0)
    {
        for (size_t i = 0; i < aNode.children.size(); ++i)
        {
            LoadNode(pNewNode, aModel.nodes[aNode.children[i]], aNode.children[i], aModel, aIndexBuffer, aVertexBuffer, aGlobalScale);
        }
    }

    // Node contains mesh data.
}

void LoadGLTF(const std::string& aFilePath, float aScale)
{
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    std::string Error;
    std::string Warning;

    bool bBinary = false;
    size_t ExtPos = aFilePath.rfind('.', aFilePath.length());
    if (ExtPos != std::string::npos)
    {
        bBinary = (aFilePath.substr(ExtPos + 1, aFilePath.length() - ExtPos) == "glb");
    }

    bool bFileLoaded = bBinary ? gltfContext.LoadBinaryFromFile(&gltfModel, &Error, &Warning, aFilePath.c_str()) : 
        gltfContext.LoadASCIIFromFile(&gltfModel, &Error, &Warning, aFilePath.c_str());

    std::vector<uint32_t> IndexBuffer;
    std::vector<sVertex> VertexBuffer;

    if(bFileLoaded)
    {
        LoadTextures(gltfModel);
        LoadMaterials(gltfModel);

        const tinygltf::Scene& Scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
        for (size_t i = 0; i < Scene.nodes.size(); ++i)
        {
            const tinygltf::Node Node = gltfModel.nodes[Scene.nodes[i]];
            LoadNode(nullptr, Node, Scene.nodes[i], gltfModel, IndexBuffer, VertexBuffer, aScale);
        }

    }
}