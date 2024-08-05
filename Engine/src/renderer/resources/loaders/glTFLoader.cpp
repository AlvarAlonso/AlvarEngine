#include "glTFLoader.hpp"

#define TINYGLTF_IMPLEMENTATION

// TODO: What does this do?
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define STBI_MSC_SECURE_CRT
#include <tinygltf/tiny_gltf.h>

#include <renderer/core/render_types.hpp>
#include <renderer/resources/texture.hpp>
#include <renderer/resources/material.hpp>
#include <core/logger.h>
#include <core/utils.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

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
    for (auto& Root : m_pRoots)
    {
        delete Root;
    }
}

struct sGLTFData
{
    std::string Filename;
    std::vector<CMeshNode*> Nodes;
    std::vector<CTexture*> Textures;
    std::vector<CMaterial*> Materials;
} LoadedData;

static void TextureFromGLTFImage(tinygltf::Image& aGltfImage)
{
    unsigned char* Buffer = nullptr;
	uint64_t BufferSize = 0;
	bool bDeleteBuffer = false;

	// Convert to rgba if it is rgb
	if (aGltfImage.component == 3)
	{
		BufferSize = aGltfImage.width * aGltfImage.height * 4;
		Buffer = new unsigned char[BufferSize];
		unsigned char* rgba = Buffer;
		unsigned char* rgb = &aGltfImage.image[0];
		for (int32_t i = 0; i < aGltfImage.width * aGltfImage.height; ++i)
		{
			for (int32_t j = 0; j < 3; ++j)
			{
				rgba[j] = rgb[j];
			}
			rgba += 4;
			rgb += 3;
		}
		bDeleteBuffer = true;
	}
	else
	{
		Buffer = &aGltfImage.image[0];
		BufferSize = aGltfImage.image.size();
	}

    // Create and upload the image in the graphics API being used.
    CTexture* pNewTexture = CTexture::Create(BufferSize, Buffer, aGltfImage.width, aGltfImage.height);
    std::string ID = aGltfImage.name;
    if (ID.empty())
    {
        ID = LoadedData.Filename.append(std::to_string(LoadedData.Textures.size()));
    }

    pNewTexture->SetID(ID);
    LoadedData.Textures.push_back(pNewTexture);
    CTexture::RegisterTexture(pNewTexture);
}

static void LoadTextures(tinygltf::Model &aGltfModel)
{
    for(tinygltf::Texture &Tex : aGltfModel.textures) 
    {
        if (aGltfModel.images.size() <= 0) continue;
        tinygltf::Image Image = aGltfModel.images[Tex.source];
        
        TextureFromGLTFImage(Image);
    }
}

static void LoadMaterials(tinygltf::Model &aGltfModel)
{
    for(tinygltf::Material &mat : aGltfModel.materials)
    {
        CMaterial* pMaterial = new CMaterial();
        sMaterialProperties Props = {};
        if(mat.values.find("baseColorTexture") != mat.values.end()) {
            CTexture* pAlbedoTexture = LoadedData.Textures[mat.values["baseColorTexture"].TextureIndex()];
            Props.pAlbedoTexture = CTexture::Get<CTexture>(pAlbedoTexture->GetID());
        }
        if(mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
            CTexture* pMetallicRoughnesTexture = LoadedData.Textures[mat.values["metallicRoughnessTexture"].TextureIndex()];
            Props.pMetallicRoughnessTexture = CTexture::Get<CTexture>(pMetallicRoughnesTexture->GetID());
        }
        if(mat.values.find("roughnessFactor") != mat.values.end()) {
            Props.MaterialConstants.RoughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
        }
        if(mat.values.find("metallicFactor") != mat.values.end()) {
            Props.MaterialConstants.MetallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
        }
        if(mat.values.find("baseColorFactor") != mat.values.end()) {
            Props.MaterialConstants.Color = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
        }
        if(mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
            CTexture* pNormalTexture = LoadedData.Textures[mat.additionalValues["normalTexture"].TextureIndex()];
            Props.pNormalTexture = CTexture::Get<CTexture>(pNormalTexture->GetID());
        }
        if(mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
            CTexture* pOcclusionTexture = LoadedData.Textures[mat.additionalValues["occlusionTexture"].TextureIndex()];
            Props.pOcclusionTexture = CTexture::Get<CTexture>(pOcclusionTexture->GetID());
        }
        if(mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end()) {
            Props.MaterialConstants.EmissiveFactor = glm::vec4(glm::make_vec3(mat.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0f);
        }

        pMaterial->SetID(mat.name.c_str());
        pMaterial->SetMaterialProperties(Props);
        LoadedData.Materials.push_back(pMaterial);
        CMaterial::RegisterMaterial(pMaterial);
    }
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
    glm::vec3 Scale = glm::vec3(aGlobalScale);
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
    if (aNode.mesh > -1)
    {
        const tinygltf::Mesh Mesh = aModel.meshes[aNode.mesh];
        sMeshData* pNewMesh = new sMeshData();
        for (size_t i = 0; i < Mesh.primitives.size(); ++i)
        {
            const tinygltf::Primitive& Primitive = Mesh.primitives[i];
            const uint32_t IndexStart = static_cast<uint32_t>(aIndexBuffer.size());
            const uint32_t VertexStart = static_cast<uint32_t>(aVertexBuffer.size());
            uint32_t IndexCount = 0;
            uint32_t VertexCount = 0;
            glm::vec3 PosMin{};
            glm::vec3 PosMax{};
            bool bHasIndices = Primitive.indices > -1;
            // Vertices.
            {
                const float* BufferPos = nullptr;
                const float* BufferNormals = nullptr;
                const float* BufferTexCoordSet0 = nullptr;

                int PosByteStride;
                int NormalByteStride;
                int UV0ByteStride;

                // Position attribute is required.
                assert(Primitive.attributes.find("POSITION") != Primitive.attributes.end());

                const tinygltf::Accessor& PosAccessor = aModel.accessors[Primitive.attributes.find("POSITION")->second];
                const tinygltf::BufferView& PosView = aModel.bufferViews[PosAccessor.bufferView];
                BufferPos = reinterpret_cast<const float*>(&aModel.buffers[PosView.buffer].data[PosAccessor.byteOffset + PosView.byteOffset]);
                PosMin = glm::vec3(PosAccessor.minValues[0], PosAccessor.minValues[1], PosAccessor.minValues[2]);
                PosMax = glm::vec3(PosAccessor.maxValues[0], PosAccessor.maxValues[1], PosAccessor.maxValues[2]);
                VertexCount = static_cast<uint32_t>(PosAccessor.count);
                PosByteStride = PosAccessor.ByteStride(PosView) ? (PosAccessor.ByteStride(PosView) / sizeof(float)) : TINYGLTF_TYPE_VEC3 * 4;

                if(Primitive.attributes.find("NORMAL") != Primitive.attributes.end())
                {
                    const tinygltf::Accessor& NormAccessor = aModel.accessors[Primitive.attributes.find("NORMAL")->second];
                    const tinygltf::BufferView& NormView = aModel.bufferViews[NormAccessor.bufferView];
                    BufferNormals = reinterpret_cast<const float*>(&(aModel.buffers[NormView.buffer].data[NormAccessor.byteOffset + NormView.byteOffset]));
                    NormalByteStride = NormAccessor.ByteStride(NormView) ? (NormAccessor.ByteStride(NormView) / sizeof(float)) : TINYGLTF_TYPE_VEC3 * 4;
                }

                if(Primitive.attributes.find("TEXCOORD_0") != Primitive.attributes.end())
                {
                    const tinygltf::Accessor& UVAccessor = aModel.accessors[Primitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView& UVView = aModel.bufferViews[UVAccessor.bufferView];
                    BufferTexCoordSet0 = reinterpret_cast<const float*>(&(aModel.buffers[UVView.buffer].data[UVAccessor.byteOffset + UVView.byteOffset]));
                    UV0ByteStride = UVAccessor.ByteStride(UVView) ? (UVAccessor.ByteStride(UVView) / sizeof(float)) : TINYGLTF_TYPE_VEC2 * 4;
                }

                for(size_t v = 0; v < PosAccessor.count; v++)
                {
                    sVertex Vert{};

                    Vert.Position = glm::vec4(glm::make_vec3(&BufferPos[v * PosByteStride]), 1.0f);
                    Vert.Normal = glm::normalize(glm::vec3(BufferNormals ? glm::make_vec3(&BufferNormals[v * NormalByteStride]) : glm::vec3(0.0f)));
                    Vert.UV = BufferTexCoordSet0 ? glm::make_vec2(&BufferTexCoordSet0[v * UV0ByteStride]) : glm::vec3(0.0f);

                    aVertexBuffer.push_back(Vert);
                }
            }

            // Indices.
            if(bHasIndices)
            {
                const tinygltf::Accessor& Accessor = aModel.accessors[Primitive.indices > -1 ? Primitive.indices : 0];
                const tinygltf::BufferView& BufferView = aModel.bufferViews[Accessor.bufferView];
                const tinygltf::Buffer& buffer = aModel.buffers[BufferView.buffer];

                IndexCount = static_cast<uint32_t>(Accessor.count);
                const void* dataPtr = &(buffer.data[Accessor.byteOffset + BufferView.byteOffset]);

                switch(Accessor.componentType)
                {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                    const uint32_t* Buf = static_cast<const uint32_t*>(dataPtr);
                    for(size_t index = 0; index < Accessor.count; index++) {
                        aIndexBuffer.push_back(Buf[index] + VertexStart);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    const uint16_t* Buf = static_cast<const uint16_t*>(dataPtr);
                    for(size_t index = 0; index < Accessor.count; index++) {
                        aIndexBuffer.push_back(Buf[index] + VertexStart);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                    const uint8_t* Buf = static_cast<const uint8_t*>(dataPtr);
                    for(size_t index = 0; index < Accessor.count; index++) {
                        aIndexBuffer.push_back(Buf[index] + VertexStart);
                    }
                    break;
                }
                default:
                    SGSERROR("Index component type %d not supported!", Accessor.componentType)
                    return;
                }
            }

            CSubMesh* pNewPrimitive = new CSubMesh(VertexStart, IndexStart, IndexCount, VertexCount, Primitive.material > -1 ? LoadedData.Materials[Primitive.material] : nullptr); // TODO: Default material instead of "nullptr".
            pNewMesh->SubMeshes.push_back(pNewPrimitive);
        }

        pNewNode->m_pMeshData = pNewMesh;
    }

    if (aParent)
    {
        aParent->m_Children.push_back(pNewNode);
    }
    else
    {
        LoadedData.Nodes.push_back(pNewNode);
    }
}

CRenderable* LoadGLTF(const std::string& aFilePath, float aScale)
{
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    std::string Error;
    std::string Warning;

    LoadedData.Filename = utils::GetFileName(aFilePath);

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

        if (LoadedData.Nodes.size() == 0)
        {
            SGSERROR("No Nodes where found while loading %s!!!", aFilePath.c_str());
            return nullptr;
        }

        CRenderable* pRenderable = CRenderable::Create();
        pRenderable->m_pRoots = LoadedData.Nodes;
        pRenderable->m_Vertices = VertexBuffer;
        pRenderable->m_Indices = IndexBuffer;
        pRenderable->m_VerticesCount = static_cast<uint32_t>(VertexBuffer.size());
        pRenderable->m_IndicesCount = static_cast<uint32_t>(IndexBuffer.size());

        LoadedData.Textures.clear();
        LoadedData.Materials.clear();
        LoadedData.Nodes.clear();

        return pRenderable;
    }

    return nullptr;
}