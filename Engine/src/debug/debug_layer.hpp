#pragma once

#include <core/ILayer.hpp>
#include <core/event_system/event_base.hpp>

namespace Alvar
{
    class CDebugLayer : public ILayer
    {
    public:
        CDebugLayer();
        ~CDebugLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate(float aDeltaTime) {}
        virtual void OnImGuiRender() {}
        virtual void OnEvent(CEvent& aEvent) override;


        void Begin();
        void End();
    };
}