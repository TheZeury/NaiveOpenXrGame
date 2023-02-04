#pragma once

#include "mainCommon.h"
#include "MeshModel.h"
#include "Texture.h"
#include "Bricks/Scene.h"
#include "XR/XrInstance.h"

namespace Noxg
{
	MAKE_HANDLE(GraphicsInstance);

	class GraphicsInstance
	{
	public:
		virtual void Initialize(rf::XrInstance xrInstance) = 0;
		virtual void CleanUpInstance() = 0;
		virtual void CleanUpSession() = 0;
		virtual void InitializeSession() = 0; 
		virtual void CreateSwapChainImageViews(std::vector<std::vector<xr::SwapchainImageVulkanKHR>>& swapChainImages, vk::Format format, std::vector<xr::Rect2Di> rects) = 0;
		virtual void RenderView(xr::CompositionLayerProjectionView projectionView, uint32_t view, uint32_t imageIndex, vk::Format format) = 0;

	public:
		virtual void addTexture(hd::Texture texture) = 0;
		virtual void addModel(hd::MeshModel model) = 0;
		virtual void addScene(rf::Scene scene) = 0;
		virtual hd::GameObject loadGameObjectFromFiles(std::string name) = 0;	// May creates multiple textures and models, but only a single gameObject.
		virtual xr::GraphicsBindingVulkanKHR getGraphicsBinding() = 0;
	};
}

