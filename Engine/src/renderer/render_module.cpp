#include "render_module.hpp"
#include "core/logger.h"

#include <glm/gtx/transform.hpp>

#include <stdexcept>

CRenderModule::CRenderModule() :
    m_pDefaultScene(nullptr),
    m_pVulkanBackend(nullptr)
{
}

bool CRenderModule::Initialize(const HINSTANCE aInstanceHandle, const HWND aWindowHandle)
{
    m_pMainCamera = new CCamera();

    CreateDefaultScene();

    m_pVulkanBackend = std::make_unique<CVulkanBackend>();
    if (m_pVulkanBackend == nullptr)
    {
        throw std::runtime_error("Failed to create the Vulkan Backend!");
    }

    m_pVulkanBackend->Initialize(aInstanceHandle, aWindowHandle);
    m_pVulkanBackend->CreateRenderObjectsData(m_pDefaultScene->GetRenderObjects());

    return true;
}

void CRenderModule::Render()
{
    // TODO: Should this pointer be checked?
    m_pVulkanBackend->Render(m_pMainCamera);
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

void CRenderModule::HandleInput(WPARAM aInput)
{
    if (m_pMainCamera)
    {
        SGSDEBUG("pMainCamera Input");
        m_pMainCamera->HandleInput(aInput);
    }
}

std::unordered_map<std::string, sRenderObjectInfo> CRenderModule::m_RenderObjectInfos{};

void CRenderModule::CreateDefaultScene()
{
    sRenderObjectInfo RenderObjectInfo("../Resources/Meshes/viking_room.obj", "../Resources/Images/viking_room.png");    
    m_RenderObjectInfos["VikingRoom"] = RenderObjectInfo;

    m_pDefaultScene = new CScene();

    glm::mat4 ModelMatrix(0);
    ModelMatrix = glm::translate(glm::vec3{0.0f, 0.0f, 0.0f});
    sRenderObject* RenderObject = new sRenderObject();
    RenderObject->ModelMatrix = ModelMatrix;
    RenderObject->pRenderObjectInfo = &m_RenderObjectInfos.at("VikingRoom");

    sRenderObject* RenderObject2 = new sRenderObject();
    RenderObject2->ModelMatrix = glm::translate(glm::vec3{10.0f, 0.0f, 10.0f});
    RenderObject2->pRenderObjectInfo = &m_RenderObjectInfos.at("VikingRoom");

    m_pDefaultScene->AddRenderObject(RenderObject);
    m_pDefaultScene->AddRenderObject(RenderObject2);
}
