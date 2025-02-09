#include "input_module.hpp"

namespace Alvar
{
    namespace Input
    {
        CInputModule::CInputModule()
        {

        }

        bool CInputModule::Initialize()
        {
            return true;
        }

        void CInputModule::Update(float aDeltaTime)
        {
            m_KeyboardData.Update(aDeltaTime);
        }

        bool CInputModule::Shutdown()
        {
            return true;
        }

        // TODO: Check validity of Keycode?
        bool CInputModule::HandleKeyPressed(CKeyPressedEvent& aEvent)
        {
            m_KeyboardData.Keys[aEvent.GetKeyCode()].Value = 1.0f; // TODO: Add sensitivity.
            return true;
        }

        bool CInputModule::HandleKeyReleased(CKeyReleasedEvent& aEvent)
        {
            m_KeyboardData.Keys[aEvent.GetKeyCode()].Value = 0.0f;
            return true;
        }

        bool CInputModule::HandleMouseMoved(CMouseMovedEvent& aEvent)
        {
            glm::vec2 OldPosition = m_MouseData.Position;
            m_MouseData.Position = glm::vec2(aEvent.GetX(), aEvent.GetY());
            m_MouseData.Delta = m_MouseData.Position - OldPosition;
            return true;
        }

        bool CInputModule::HandleMouseScrolled(CMouseScrolledEvent& aEvent)
        {
            return true;
        }

        bool CInputModule::HandleMouseButtonPressed(CMouseButtonPressedEvent& aEvent)
        {
            m_MouseData.Buttons[aEvent.GetMouseButton()].Value = 1.0f;
            return true;
        }

        bool CInputModule::HandleMouseButtonReleased(CMouseButtonReleasedEvent& aEvent)
        {
            m_MouseData.Buttons[aEvent.GetMouseButton()].Value = 0.0f;
            return true;
        }
    }
}
