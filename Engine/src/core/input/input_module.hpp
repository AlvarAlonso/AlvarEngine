#pragma once

#include <core/IModule.hpp>

namespace Alvar
{
    class CInputModule : public IModule
    {
    public:
        CInputModule();
        CInputModule(const CInputModule&) = delete;

        virtual bool Initialize() override;
        virtual void Update() override;
        virtual bool Shutdown() override;
    };
}
