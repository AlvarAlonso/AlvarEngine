#pragma once

#include <core/event_system/event_base.hpp>
#include <core/input/input_codes.hpp>
#include <sstream>

namespace Alvar
{
    // WINDOW EVENTS.
    class CWindowResizeEvent : public CEvent
    {
    public:
        CWindowResizeEvent(uint32_t aWidth, uint32_t aHeight) :
            m_Width(aWidth), m_Height(aHeight){}

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }

        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
       
    private:
        uint32_t m_Width;
        uint32_t m_Height;
    };

    class CWindowCloseEvent : public CEvent
    {
    public:
        CWindowCloseEvent() = default;

        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)    
    };

    class CAppTickEvent : public CEvent
    {
    public:
        CAppTickEvent() = default;

        EVENT_CLASS_TYPE(AppTick)
        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
    };

    class CAppUpdateEvent : public CEvent
    {
    public:
        CAppUpdateEvent() = default;

        EVENT_CLASS_TYPE(AppUpdate)
        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
    };

    class CAppRenderEvent : public CEvent
    {
    public:
        CAppRenderEvent() = default;

        EVENT_CLASS_TYPE(AppRender)
        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_APPLICATION)
    };

    // KEYBOARD EVENTS.
    class CKeyEvent : public CEvent
    {
    public:
        KeyCode GetKeyCode() const { return m_KeyCode; }

        EVENT_CLASS_CATEGORY(EVENT_CATEGORY_KEYBOARD | EVENT_CATEGORY_INPUT)
    protected:
        CKeyEvent(const KeyCode aKeyCode) :
            m_KeyCode(aKeyCode) {}

        KeyCode m_KeyCode;
    };

    class CKeyPressedEvent : public CKeyEvent
    {
    public:
        CKeyPressedEvent(const KeyCode aKeyCode, bool bIsRepeat = false) :
            CKeyEvent(aKeyCode), m_IsRepeat(bIsRepeat) {}
        
        bool IsRepeat() const { return m_IsRepeat; }

        EVENT_CLASS_TYPE(KeyPressed)

    private:
        bool m_IsRepeat;
    };

	class CKeyReleasedEvent : public CKeyEvent
	{
	public:
		CKeyReleasedEvent(const KeyCode aKeycode) : 
            CKeyEvent(aKeycode) {}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class CKeyTypedEvent : public CKeyEvent
	{
	public:
		CKeyTypedEvent(const KeyCode aKeycode) : 
            CKeyEvent(aKeycode) {}

		EVENT_CLASS_TYPE(KeyTyped)
	};

    // MOUSE EVENTS.
    class CMouseMovedEvent : public CEvent
	{
	public:
		CMouseMovedEvent(const float x, const float y)
			: m_MouseX(x), m_MouseY(y) {}

		float GetX() const { return m_MouseX; }
		float GetY() const { return m_MouseY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT)
	private:
		float m_MouseX, m_MouseY;
	};

	class CMouseScrolledEvent : public CEvent
	{
	public:
		CMouseScrolledEvent(const float xOffset, const float yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset) {}

		float GetXOffset() const { return m_XOffset; }
		float GetYOffset() const { return m_YOffset; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << GetXOffset() << ", " << GetYOffset();
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT)
	private:
		float m_XOffset;
        float m_YOffset;
	};

	class CMouseButtonEvent : public CEvent
	{
	public:
		MouseCode GetMouseButton() const { return m_Button; }

		EVENT_CLASS_CATEGORY(EVENT_CATEGORY_MOUSE | EVENT_CATEGORY_INPUT | EVENT_CATEGORY_MOUSE_BUTTON)
	protected:
		CMouseButtonEvent(const MouseCode aButton)
			: m_Button(aButton) {}

		MouseCode m_Button;
	};

	class CMouseButtonPressedEvent : public CMouseButtonEvent
	{
	public:
		CMouseButtonPressedEvent(const MouseCode aButton) : 
            CMouseButtonEvent(aButton) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class CMouseButtonReleasedEvent : public CMouseButtonEvent
	{
	public:
		CMouseButtonReleasedEvent(const MouseCode aButton) : 
            CMouseButtonEvent(aButton) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}
