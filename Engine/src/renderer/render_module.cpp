#include "render_module.hpp"
#include "core/logger.h"
#include "engine.hpp"
#include <renderer/resources/material.hpp>
#include <renderer/resources/texture.hpp>
#include <renderer/scene/entities/light_source.hpp>
#include <renderer/resources/loaders/glTFLoader.hpp>
#include <renderer/controllers/editor_camera_controller.hpp>
#include <renderer/core/camera.hpp>

#include <glm/gtx/transform.hpp>
#include <GLFW/glfw3.h>

#include <stdexcept>

namespace Alvar
{
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
        m_pMainCamera = std::make_shared<CCamera>();
        m_pEditorCameraController = std::make_shared<CEditorCameraController>();
        m_pEditorCameraController->SetCamera(m_pMainCamera);

        m_pVulkanBackend = std::make_unique<CVulkanBackend>();
        if (m_pVulkanBackend == nullptr)
        {
            throw std::runtime_error("Failed to create the Vulkan Backend!");
        }

        m_pVulkanBackend->Initialize();
        CreateDefaultScene();
        m_pVulkanBackend->CreateRenderablesData(m_pDefaultScene);

        return true;
    }

    void CRenderModule::Update(float aDeltaTime)
    {
        m_pEditorCameraController->Update(aDeltaTime);
        
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

    void CRenderModule::InitImGuiBackend()
    {
        m_pVulkanBackend->InitImGuiBackend();
    }

    void CRenderModule::ShutdownImGuiBackend()
    {
        m_pVulkanBackend->ShutdownImGuiBackend();
    }

    void CRenderModule::ImGuiBeginFrame()
    {
        m_pVulkanBackend->ImGuiBeginFrame();
    }

    void CRenderModule::ImGuiEndFrame()
    {
        m_pVulkanBackend->ImGuiEndFrame();
    }

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

        sLightSource* LightSource = new sLightSource();
        //LightSource->Properties.Model = glm::translate(glm::vec3{2.0f, 0.0f, 4.0f});
        LightSource->Properties.Color = {0.0f, 255.0f, 0.0f};
        LightSource->Properties.Intensity = 1.0f;
        LightSource->Properties.LightType = eLightType::DIRECTIONAL;
        LightSource->Properties.TargetPosition = {0.0f, 0.0f, 0.0f};
        LightSource->Properties.MaxDist = 20.0f;

        sLightSource* LightSource2 = new sLightSource();
        //LightSource2->Properties.Model = glm::translate(glm::vec3{2.0f, 4.0f, 0.0f});
        LightSource2->Properties.Color = {255.0f, 0.0f, 0.0f};
        LightSource2->Properties.Intensity = 1.0f;
        LightSource2->Properties.LightType = eLightType::POINT;
        LightSource2->Properties.TargetPosition = {0.0f, 0.0f, 0.0f};
        LightSource2->Properties.MaxDist = 30.0f;

        m_pDefaultScene->AddLightSource(LightSource);
        m_pDefaultScene->AddLightSource(LightSource2);
    }
}
