#pragma once

#include "mainCommon.h"
#include "VulkanInstance.h"

namespace Noxg
{
	class OpenXrInstance
	{
	public:
		OpenXrInstance(VulkanInstance& vulkan);
		~OpenXrInstance();

		void Initialize();
		void CreateInstance();
		void InitializeSystem();
		void InitializeSession();
		void CreateSession(xr::GraphicsBindingVulkanKHR graphicsBinding);
		void CreateSpace();
		void CreateSwapChains();
		bool PollEvents();
		bool HandleSessionStateChangedEvent(xr::EventDataSessionStateChanged eventDataSessionStateChanged);
		bool running() { return sessionRunning; }
		void Update();
		const xr::Instance& getInstance() const;
		const xr::SystemId& getSystemId() const;
	private:
		xr::Instance instance;
		xr::Session session;
		xr::Space appSpace;
		xr::SystemId systemId;
		std::vector<xr::Swapchain> swapChains;
		std::vector<xr::Rect2Di> swapChainRects;
		std::vector<std::vector<xr::SwapchainImageVulkanKHR>> swapChainImages;
		std::vector<xr::ViewConfigurationView> configViews;
		vk::Format swapChainFormat;
		xr::EventDataBuffer eventDataBuffer;
		xr::SessionState sessionState;
		bool sessionRunning;
	private:
		VulkanInstance& graphics;
	};
}

