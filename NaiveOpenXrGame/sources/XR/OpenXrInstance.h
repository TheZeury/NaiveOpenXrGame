#pragma once

#include "mainCommon.h"
#include "Renderer/VulkanInstance.h"
#include "Utils.h"

namespace Noxg
{
	struct InputState
	{
		xr::ActionSet actionSet;
		xr::Action poseAction;
		std::array<xr::Path, 2> handSubactionPath;
		std::array<xr::Space, 2> handSpace;
		std::array<xr::Bool32, 2> handActive;
	};

	class OpenXrInstance
	{
	public:
		OpenXrInstance(VulkanInstance& vulkan);
		~OpenXrInstance();

		void Initialize();
		void CleanUpInstance();
		void CleanUpSession();
		void CreateInstance();
		void InitializeSystem();
		void InitializeSession();
		void CreateSession(xr::GraphicsBindingVulkanKHR graphicsBinding);
		void CreateSpace();
		void CreateSwapChains();
		void CreateActions();
		bool PollEvents();
		bool HandleSessionStateChangedEvent(xr::EventDataSessionStateChanged eventDataSessionStateChanged);
		bool running() { return sessionRunning; }
		void PoolActions();
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

		InputState inputState;
	private:
		VulkanInstance& graphics;
	};
}

