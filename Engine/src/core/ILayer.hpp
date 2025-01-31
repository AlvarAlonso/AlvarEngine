#pragma once

#include <string>

namespace Alvar {

    class CEvent;

	class ILayer
	{
	public:
		ILayer(const std::string& aName) :
            m_DebugName(aName){}
		virtual ~ILayer() = default;

		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void OnUpdate(float aDeltaTime) = 0;
		virtual void OnImGuiRender() = 0;
		virtual void OnEvent(CEvent& aEvent) = 0;

		const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}