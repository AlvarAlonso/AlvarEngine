#include "render_module.hpp"

#include <stdexcept>

CRenderModule::CRenderModule() :
    m_pDefaultScene(nullptr),
    m_pVulkanBackend(nullptr)
{
}

bool CRenderModule::Initialize(const HINSTANCE aInstanceHandle, const HWND aWindowHandle)
{
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
    m_pVulkanBackend->Render();
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

std::unordered_map<std::string, sRenderObjectInfo> CRenderModule::m_RenderObjectInfos{};

void CRenderModule::CreateDefaultScene()
{
    sRenderObjectInfo RenderObjectInfo("../Resources/Meshes/viking_room.obj", "../Resources/Images/viking_room.png");    
    m_RenderObjectInfos["VikingRoom"] = RenderObjectInfo;

    m_pDefaultScene = new CScene();

    glm::mat4 ModelMatrix(0);
    sRenderObject* RenderObject = new sRenderObject();
    RenderObject->ModelMatrix = ModelMatrix;
    RenderObject->pRenderObjectInfo = &m_RenderObjectInfos.at("VikingRoom");

    m_pDefaultScene->AddRenderObject(RenderObject);
}
