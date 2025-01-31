#include "layer_stack.hpp"

namespace Alvar {

	CLayerStack::~CLayerStack()
	{
		for (ILayer* Layer : m_Layers)
		{
			Layer->OnDetach();
			delete Layer;
		}
	}

	void CLayerStack::PushLayer(ILayer* aLayer)
	{
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, aLayer);
		m_LayerInsertIndex++;
	}

	void CLayerStack::PushOverlay(ILayer* aOverlay)
	{
		m_Layers.emplace_back(aOverlay);
	}

	void CLayerStack::PopLayer(ILayer* aLayer)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, aLayer);
		if (it != m_Layers.begin() + m_LayerInsertIndex)
		{
			aLayer->OnDetach();
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	}

	void CLayerStack::PopOverlay(ILayer* aOverlay)
	{
		auto it = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), aOverlay);
		if (it != m_Layers.end())
		{
			aOverlay->OnDetach();
			m_Layers.erase(it);
		}
	}
}
