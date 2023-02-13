#pragma once

#include "mainCommon.h"
#include "XrInstance.h"
#include "Renderer/GraphicsInstance.h"
#include "Utils.h"

namespace Noxg
{
	struct InputAction
	{
		xr::ActionSet actionSet;
		xr::Action poseAction;
		xr::Action triggerAction;
		xr::Action gripAction;
		xr::Action primaryButtonAction;
		xr::Action secondaryButtonAction;
		xr::Action thumbstickXAction;
		xr::Action thumbstickYAction;
		xr::Action vibrateAction;
		std::array<xr::Path, 2> handSubactionPath;
		std::array<xr::Space, 2> handSpace;
	};

	struct InputState
	{
		std::array<xr::Bool32, 2> handActive;
		std::array<xr::SpaceLocation, 2> handLocations;
		std::array<xr::ActionStateFloat, 2> triggerStates;
		std::array<xr::ActionStateFloat, 2> gripStates;
		std::array<xr::ActionStateBoolean, 2> primaryButtonStates;
		std::array<xr::ActionStateBoolean, 2> secondaryButtonStates;
		std::array<xr::ActionStateFloat, 2> thumbstickXStates;
		std::array<xr::ActionStateFloat, 2> thumbstickYStates;
	};

	MAKE_HANDLE(OpenXrInstance);

	class OpenXrInstance : public XrInstance
	{
	public:
		OpenXrInstance(rf::GraphicsInstance vulkan);
		~OpenXrInstance();

		virtual void Initialize() override;
		virtual void CleanUpInstance() override;
		virtual void CleanUpSession() override;
		void CreateInstance();
		void InitializeSystem();
		virtual void InitializeSession() override;
		void CreateSession(xr::GraphicsBindingVulkanKHR graphicsBinding);
		void CreateSpace();
		void CreateSwapChains();
		void CreateActions();
		virtual bool PollEvents() override;
		bool HandleSessionStateChangedEvent(xr::EventDataSessionStateChanged eventDataSessionStateChanged);
		virtual bool running() override { return sessionRunning; }
		virtual void PollActions() override;
		virtual void Update() override;
		const xr::Instance& getInstance() const;
		const xr::SystemId& getSystemId() const;
	public:
		virtual void vibrate(const xr::HapticVibration& virbation, int hand) override;
		InputState inputState;
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

		InputAction inputAction;
	private:
		rf::GraphicsInstance graphics;
	};
}

