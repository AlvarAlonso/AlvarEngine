#pragma once

#include <core/IModule.hpp>

#include <vector>

namespace Alvar
{
    class CModuleManager
    {
    public:
        CModuleManager() = default;

        void RegisterModule(IModule* apModule);
        void ClearModules();
    private:
        void StopModules();

        std::vector<IModule*> m_Modules;
    };
}
