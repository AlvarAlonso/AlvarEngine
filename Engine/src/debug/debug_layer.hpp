#pragma once

#include <core/ILayer.hpp>

namespace Alvar
{
    class CEvent;

    class CDebugLayer : public ILayer
    {
    public:
        CDebugLayer();
        ~CDebugLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate(float aDeltaTime) override {};
        virtual void OnImGuiRender() {}
        virtual void OnEvent(CEvent& aEvent) override;
    
    private:
        bool m_BlockEvents;
    };
}