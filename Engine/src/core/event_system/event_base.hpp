#pragma once

#include <string>

namespace Alvar
{
    enum class eEventType
    {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
        AppTick, AppUpdate, AppRender,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
        NUM
    };

    enum eEventCategory : int
    {
        EVENT_CATEGORY_NONE = 0,
        EVENT_CATEGORY_APPLICATION       = 0x00001,
        EVENT_CATEGORY_INPUT             = 0x00010,
        EVENT_CATEGORY_KEYBOARD          = 0x00100,
        EVENT_CATEGORY_MOUSE             = 0x01000,
        EVENT_CATEGORY_MOUSE_BUTTON      = 0x10000,
        EVENT_CATEGORY_NUM
    };

#define EVENT_CLASS_TYPE(type) static eEventType GetStaticType() { return eEventType::type; }\
                                virtual eEventType GetEventType() const override { return GetStaticType(); }\
                                virtual const std::string GetName() const override { return std::string(#type); }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

    /**
     * @brief Base class for all events.
     * Child classes add information related to the particular event.
     */
    class CEvent 
    {
    public:
        virtual ~CEvent() = default;

        bool bHandled {false};

        virtual eEventType GetEventType() const = 0;
        virtual const std::string GetName() const = 0;
        virtual int GetCategoryFlags() const = 0;
        virtual std::string ToString() const { return GetName(); }

        bool InInCategory(eEventCategory aCategory)
        {
            return GetCategoryFlags() & static_cast<uint8_t>(aCategory);
        }
    };

    class CEventDispatcher
    {
    public:
        CEventDispatcher(CEvent& aEvent)
            : m_Event(aEvent)
        {}

        template<typename T, typename F>
        bool Dispatch(const F& aFunc)
        {
            if (m_Event.GetEventType() == T::GetStaticType())
            {
                m_Event.bHandled |= aFunc(static_cast<T&>(m_Event));
            }

            return false;
        }

    private:
        CEvent& m_Event;
    };

    inline std::ostream& operator<<(std::ostream& os, const CEvent& e)
	{
		return os << e.ToString();
	}
}
