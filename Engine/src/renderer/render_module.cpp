#include "render_module.hpp"
#include "core/logger.h"
#include "engine.hpp"
#include <renderer/resources/material.hpp>
#include <renderer/resources/texture.hpp>

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
    m_pVulkanBackend->CreateRenderObjectsData(m_pDefaultScene->GetRenderObjects());

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
    sRenderObjectInfo RenderObjectInfo("../Resources/Meshes/viking_room.obj", "../Resources/Images/viking_room.png");    
    m_RenderObjectInfos["VikingRoom"] = RenderObjectInfo;

    sRenderObjectInfo RenderObjectInfo2("sphere", "");
    RenderObjectInfo2.MaterialName = "test_material";
    m_RenderObjectInfos["Sphere"] = RenderObjectInfo2;

    m_pDefaultScene = new CScene();

    // glm::mat4 ModelMatrix(0);
    // ModelMatrix = glm::translate(glm::vec3{0.0f, 0.0f, 0.0f});
    // sRenderObject* RenderObject = new sRenderObject();
    // RenderObject->ModelMatrix = ModelMatrix;
    // RenderObject->pRenderObjectInfo = &m_RenderObjectInfos.at("VikingRoom");

    // sRenderObject* RenderObject2 = new sRenderObject();
    // RenderObject2->ModelMatrix = glm::translate(glm::vec3{10.0f, 0.0f, 10.0f});
    // RenderObject2->pRenderObjectInfo = &m_RenderObjectInfos.at("VikingRoom");

    sRenderObject* RenderObject3 = new sRenderObject();
    RenderObject3->ModelMatrix = glm::translate(glm::vec3(1.0f, 1.0f, -3.0f));
    RenderObject3->pRenderObjectInfo = &m_RenderObjectInfos.at("Sphere");

    // m_pDefaultScene->AddRenderObject(RenderObject);
    // m_pDefaultScene->AddRenderObject(RenderObject2);
    m_pDefaultScene->AddRenderObject(RenderObject3);

    const auto& DefaultTexture = CTexture::Get<CTexture>("../Resources/Images/default_texture.png");

    CMaterial* pDefaultMaterial = new CMaterial();
    sMaterialProperties Props;
    Props.pAlbedoTexture = DefaultTexture;
    Props.pMetallicRoughnessTexture = DefaultTexture;
    Props.pEmissiveTexture = DefaultTexture;
    Props.pNormalTexture = DefaultTexture;
    pDefaultMaterial->SetMaterialProperties(Props);

    CMaterial::Add("default_material", pDefaultMaterial);

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

    CMaterial::Add("test_material", pTestMaterial);
}
