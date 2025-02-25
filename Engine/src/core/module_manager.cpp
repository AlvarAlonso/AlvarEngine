#include "module_manager.hpp"

namespace Alvar
{
    void CModuleManager::RegisterModule(IModule* apModule)
    {
        apModule->Initialize();
    }

    void CModuleManager::ClearModules()
    {
        StopModules();
        m_Modules.clear();
    }

    void CModuleManager::StopModules()
    {
        for (auto Module : m_Modules)
        {
            Module->Shutdown();
        }
    }
}
