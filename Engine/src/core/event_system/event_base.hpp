#pragma once

#include <core/defines.h>
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

    enum eEventCategory
    {
        EVENT_CATEGORY_NONE = 0,
        EVENT_CATEGORY_APPLICATION       = BIT(0),
        EVENT_CATEGORY_INPUT             = BIT(1),
        EVENT_CATEGORY_KEYBOARD          = BIT(2),
        EVENT_CATEGORY_MOUSE             = BIT(3),
        EVENT_CATEGORY_MOUSE_BUTTON      = BIT(4),
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

        bool IsInCategory(eEventCategory aCategory)
        {
            return GetCategoryFlags() & aCategory;
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
