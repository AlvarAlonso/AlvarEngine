#pragma once

#include <core/ILayer.hpp>

#include <vector>

namespace Alvar {

	class CLayerStack
	{
	public:
		CLayerStack() = default;
		~CLayerStack();

		void PushLayer(ILayer* aLayer);
		void PushOverlay(ILayer* aOverlay);
		void PopLayer(ILayer* aLayer);
		void PopOverlay(ILayer* aOverlay);

		std::vector<ILayer*>::iterator begin() { return m_Layers.begin(); }
		std::vector<ILayer*>::iterator end() { return m_Layers.end(); }
		std::vector<ILayer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
		std::vector<ILayer*>::reverse_iterator rend() { return m_Layers.rend(); }

		std::vector<ILayer*>::const_iterator begin() const { return m_Layers.begin(); }
		std::vector<ILayer*>::const_iterator end()	const { return m_Layers.end(); }
		std::vector<ILayer*>::const_reverse_iterator rbegin() const { return m_Layers.rbegin(); }
		std::vector<ILayer*>::const_reverse_iterator rend() const { return m_Layers.rend(); }
	private:
		std::vector<ILayer*> m_Layers;
		unsigned int m_LayerInsertIndex = 0;
	};
}