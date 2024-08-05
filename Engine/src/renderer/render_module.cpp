#include "render_module.hpp"
#include "core/logger.h"
#include "engine.hpp"
#include <renderer/resources/material.hpp>
#include <renderer/resources/texture.hpp>
#include <renderer/resources/loaders/glTFLoader.hpp>

#include <glm/gtx/transform.hpp>
#include <GLFW/glfw3.h>

#include <stdexcept>

CRenderModule::CRenderModule() :
    m_pMainCamera(nullptr),
    m_pDefaultScene(nullptr),
    m_pVulkanBackend(nullptr),
    m_CurrentRenderPath(eRenderPath::FORWARD),
    m_RenderAPI(eRenderAPI::VULKAN)
{
}

bool CRenderModule::Initialize()
{
    m_pMainCamera = new CCamera();

    m_pVulkanBackend = std::make_unique<CVulkanBackend>();
    if (m_pVulkanBackend == nullptr)
    {
        throw std::runtime_error("Failed to create the Vulkan Backend!");
    }

    m_pVulkanBackend->Initialize();
    CreateDefaultScene();
    m_pVulkanBackend->CreateRenderablesData(m_pDefaultScene->GetRenderObjects());

    return true;
}

void CRenderModule::Update()
{
    m_pMainCamera->Update();
    
    // TODO: Release old render path resources.
    // Initialize new render path resources.
    // If something went wrong, initialize render module again.
    if (glfwGetKey(CEngine::Get()->GetWindow(), GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        m_CurrentRenderPath = m_CurrentRenderPath == eRenderPath::FORWARD ? eRenderPath::DEFERRED : eRenderPath::FORWARD;
        SGSINFO("Switching RenderPath to: %s.", m_CurrentRenderPath == eRenderPath::FORWARD ? "FORWARD" : "DEFERRED");
        m_pVulkanBackend->ChangeRenderPath();
    }

    Render();
}

bool CRenderModule::Shutdown()
{
    delete m_pMainCamera;
    delete m_pDefaultScene;
    return m_pVulkanBackend->Shutdown();
}

void CRenderModule::HandleWindowResize()
{
    m_pVulkanBackend->HandleWindowResize();
}

eRenderPath CRenderModule::GetRenderPath()
{
    return m_CurrentRenderPath;
}

void CRenderModule::Render()
{
    // TODO: Should this pointer be checked?
    m_pVulkanBackend->Render(m_pMainCamera);
}

std::unordered_map<std::string, sRenderObjectInfo> CRenderModule::m_RenderObjectInfos{};

void CRenderModule::CreateDefaultScene()
{
    // Add materials.
    const auto& DefaultTexture = CTexture::Get<CTexture>("../Resources/Images/default_texture.png");

    CMaterial* pDefaultMaterial = new CMaterial();
    sMaterialProperties Props;
    Props.pAlbedoTexture = DefaultTexture;
    Props.pMetallicRoughnessTexture = DefaultTexture;
    Props.pEmissiveTexture = DefaultTexture;
    Props.pNormalTexture = DefaultTexture;
    pDefaultMaterial->SetMaterialProperties(Props);
    pDefaultMaterial->SetID("default_material");

    CMaterial::RegisterMaterial(pDefaultMaterial);

    CMaterial* pTestMaterial = new CMaterial();
    Props.MaterialConstants.Color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    Props.MaterialConstants.EmissiveFactor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    Props.MaterialConstants.MetallicFactor = 0.5f;
    Props.MaterialConstants.RoughnessFactor = 0.5f;
    Props.MaterialConstants.TillingFactor = 1.0f;
    Props.pAlbedoTexture = CTexture::Get<CTexture>("../Resources/Images/Material_33_baseColor.png");
    Props.pMetallicRoughnessTexture = CTexture::Get<CTexture>("../Resources/Images/Material_33_metallicRoughness.png");
    Props.pEmissiveTexture = CTexture::Get<CTexture>("../Resources/Images/Material_33_emissive.png");
    Props.pNormalTexture = CTexture::Get<CTexture>("../Resources/Images/Material_33_normal.png");
    pTestMaterial->SetMaterialProperties(Props);
    pTestMaterial->SetID("test_material");

    CMaterial::RegisterMaterial(pTestMaterial);

    // CRenderable* pSphere = CRenderable::Create();
    // CMeshNode* pSphereNode = new CMeshNode();
    // pSphereNode->m_pMeshData = sMeshData::GetMeshData("sphere");
    // pSphereNode->m_Model = glm::translate(glm::vec3(1.0f, 1.0f, -3.0f));
    // pSphere->m_Vertices = std::move(pSphereNode->m_pMeshData->Vertices);
    // pSphere->m_Indices = std::move(pSphereNode->m_pMeshData->Indices32);
    // CSubMesh* pSubMesh = new CSubMesh(0, 0, pSphere->m_Indices.size(), pSphere->m_Vertices.size(), CMaterial::Get("test_material"));
    // pSphereNode->m_pMeshData->SubMeshes.push_back(pSubMesh);
    // pSphere->UploadToVRAM();
    // pSphere->m_pRoots.push_back(pSphereNode);

    CRenderable* pPato = LoadGLTF("../Resources/Prefabs/Duck.glb", 0.1f);
    pPato->UploadToVRAM();

    m_pDefaultScene = new CScene();
    //m_pDefaultScene->AddRenderable(pSphere);
    m_pDefaultScene->AddRenderable(pPato);
}
