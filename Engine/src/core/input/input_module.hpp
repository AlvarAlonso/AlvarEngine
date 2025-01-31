#pragma once

#include <core/IModule.hpp>
#include <core/input/input_codes.hpp>
#include <core/event_system/application_events.hpp>

#include <glm/vec2.hpp>
#include <array>

namespace Alvar
{
    namespace Input
    {
        struct sButtonState
        {
            static constexpr float UNPRESSED_VALUE = 0.0f;

            float Value = UNPRESSED_VALUE;
            float PreviousValue = UNPRESSED_VALUE;
            float Time = 0.0f;

            void Update(float aDeltaTime)
            {
                if ((IsPressed()))
                {
                    Time += aDeltaTime;
                }
                else
                {
                    Time = 0.0f;
                }
            }

            bool IsPressed() const
            {
                return Value != 0.0f;
            }

            bool WasPressed() const
            {
                return PreviousValue != 0.0f;
            }

            bool GetsPressed() const
            {
                return !WasPressed() && IsPressed();
            }

            bool GetsReleased() const
            {
                return WasPressed() && !IsPressed();
            }
        };

        enum class eMouseButton
        {
            Left = 0,
            Middle,
            Right,
            Num
        };

        struct sMouseData
        {
            sButtonState Buttons[3];
            glm::vec2 Position = glm::vec2(0.0f);
            glm::vec2 Delta = glm::vec2(0.0f);
            int WheelSteps = 0;
        };

        constexpr int NUM_KEYBOARD_KEYS = 256;
        using KeyboardKey = int;

        struct sKeyboardData
        {
            std::array<sButtonState, NUM_KEYBOARD_KEYS> Keys = {};

            void Update(float aDeltaTime)
            {
                for (auto& Key : Keys)
                {
                    Key.Update(aDeltaTime);
                }
            }

            sButtonState& GetButtonState(KeyCode aKeyCode)
            {
                const int Index = Key::KeyCodeMap[aKeyCode].second;
                return Keys[Index];
            }
        };

        class CInputModule : public IModule
        {
        public:
            CInputModule();
            CInputModule(const CInputModule&) = delete;

            virtual bool Initialize() override;
            virtual void Update(float aDeltaTime) override;
            virtual bool Shutdown() override;

            bool GetButtonDown(KeyCode aKeyCode) const { m_KeyboardData.Keys[aKeyCode].GetsPressed(); }
            bool GetButton(KeyCode aKeyCode) const { m_KeyboardData.Keys[aKeyCode].IsPressed(); }
            bool GetButtonUp(KeyCode aKeyCode) const { m_KeyboardData.Keys[aKeyCode].GetsReleased(); }

            bool GetMouseButtonDown(MouseCode aMouseCode) const { m_MouseData.Buttons[aMouseCode].GetsPressed(); }
            bool GetMouseButton(MouseCode aMouseCode) const { m_MouseData.Buttons[aMouseCode].IsPressed(); }
            bool GetMouseButtonUp(MouseCode aMouseCode) const { m_MouseData.Buttons[aMouseCode].GetsReleased(); }
            glm::vec2 GetMousePosition() const { m_MouseData.Position; }

            // TODO: This should not be public. Should only accessible by the class that manages the events. Use friend class?
            bool HandleKeyPressed(CKeyPressedEvent& aEvent);
            bool HandleKeyReleased(CKeyReleasedEvent& aEvent);
            bool HandleMouseMoved(CMouseMovedEvent& aEvent);
            bool HandleMouseScrolled(CMouseScrolledEvent& aEvent);
            bool HandleMouseButtonPressed(CMouseButtonPressedEvent& aEvent);
            bool HandleMouseButtonReleased(CMouseButtonReleasedEvent& aEvent);

        private:
            sKeyboardData m_KeyboardData;
            sMouseData m_MouseData;
        };    
    }
}
