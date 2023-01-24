#include "OpenXrInstance.h"

Noxg::OpenXrInstance::OpenXrInstance(VulkanInstance& vulkan) : graphics(vulkan)
{
}

Noxg::OpenXrInstance::~OpenXrInstance()
{
	for (auto& swapchain : swapChains)
	{
		swapchain.destroy();
	}
	if (appSpace != nullptr) appSpace.destroy();
	if (session != nullptr) session.destroy();
	if (instance != nullptr) instance.destroy();
}

void Noxg::OpenXrInstance::Initialize()
{
	CreateInstance();

	InitializeSystem();	
}

void Noxg::OpenXrInstance::CreateInstance()
{
	// Logging Extensions and Layers -----------------------------------------------------
	LOG_STEP("OpenXR", "Enumerating Supported OpenXR Extensions");
	auto allExtensions = xr::enumerateInstanceExtensionPropertiesToVector(nullptr);
	LOG_SUCCESS();
	LOG_INFO("OpenXR", std::format("List of total {} OpenXR Extension(s):", allExtensions.size()), 0);
	for (const auto& ext : allExtensions)
	{
		LOG_INFO("OpenXR", ext.extensionName, 1);
	}

	LOG_STEP("OpenXR", "Enumerating API Layers");
	auto layers = xr::enumerateApiLayerPropertiesToVector();
	LOG_SUCCESS();
	LOG_INFO("OpenXR", std::format("List of total {} API Layer(s):", layers.size()), 0);
	for (const auto& layer : layers)
	{
		LOG_INFO("OpenXR", layer.layerName, 1);
	}

	// Creating Instance ------------------------------------------------------------------
	if (instance)
	{
		throw std::runtime_error("Can't create a new OpenXR instance when there's already one!");
	}

	std::vector<const char*> extensions;
	extensions.push_back(XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME);
	LOG_INFO("OpenXR", std::format("List of total {} required Extension(s)", extensions.size()), 0);
	for (const auto& ext : extensions)
	{
		LOG_INFO("OpenXR", ext, 1);
	}

	LOG_STEP("OpenXR", "Creating OpenXR Instance");
	xr::ApplicationInfo appInfo(
		"Naive OpenXR Game",
		1,
		"No Engine",
		1,
		xr::Version::current()
	);

	xr::InstanceCreateFlags flags = { };
	xr::InstanceCreateInfo instanceCreateInfo(
		{ },
		appInfo,
		0,
		nullptr,
		extensions.size(),
		extensions.data()
	);

	instance = xr::createInstance(instanceCreateInfo);
	LOG_SUCCESS();

	// Logging Instance Properties -------------------------------------------------------
	auto instanceProperty = instance.getInstanceProperties();
	LOG_INFO(
		"OpenXR",
		std::format("OpenXR Runtime = {}, RuntimeVersion = {}.{}.{}",
			instanceProperty.runtimeName,
			instanceProperty.runtimeVersion.major(),
			instanceProperty.runtimeVersion.minor(),
			instanceProperty.runtimeVersion.patch()
		),
		0
	);
}

void Noxg::OpenXrInstance::InitializeSystem()
{
	LOG_STEP("OpenXR", "Initializing OpenXR System");
	
	if (!instance)
	{
		throw std::runtime_error("Can't initialize OpenXR system when there's no instance.");
	}
	if (systemId)
	{
		throw std::runtime_error("Can't initialize OpenXR system when there's already one.");
	}

	xr::SystemGetInfo systemGetInfo(
		xr::FormFactor::HeadMountedDisplay
	);

	systemId = instance.getSystem(systemGetInfo);

	LOG_SUCCESS();
}

void Noxg::OpenXrInstance::InitializeSession()
{
	CreateSession(graphics.getGraphicsBinding());
	CreateSpace();
	CreateSwapChains();
}

void Noxg::OpenXrInstance::CreateSession(xr::GraphicsBindingVulkanKHR graphicsBinding)
{
	LOG_STEP("OpenXR", "Creating OpenXR Session");
	xr::SessionCreateInfo createInfo({}, systemId, &graphicsBinding);
	session = instance.createSession(createInfo);
	LOG_SUCCESS();
}

void Noxg::OpenXrInstance::CreateSpace()
{
	LOG_STEP("OpenXR", "Creating Reference Space");
	xr::ReferenceSpaceCreateInfo refSpaceCreateInfo(xr::ReferenceSpaceType::Local, { });
	appSpace = session.createReferenceSpace(refSpaceCreateInfo);
	LOG_SUCCESS();
}

void Noxg::OpenXrInstance::CreateSwapChains()
{
	if (!systemId)
	{
		throw std::runtime_error("No system ID.");
	}

	auto systemPorperties = instance.getSystemProperties(systemId);
	LOG_INFO("OpenXR", "System Properties: ", 0);
	LOG_INFO("OpenXR", std::format("System Name : {}, Vendor ID : {}", systemPorperties.systemName, systemPorperties.vendorId), 1);
	LOG_INFO("OpenXR", std::format("Graphics Properties: MaxSwapChainImageWidth = {}, MaxSwapChainImageHeight = {}, MaxLayers = {}", systemPorperties.graphicsProperties.maxSwapchainImageWidth, systemPorperties.graphicsProperties.maxSwapchainImageHeight, systemPorperties.graphicsProperties.maxLayerCount), 1);
	LOG_INFO("OpenXR", std::format("Tracking Properties: OrientationTracking = {}, PositionTracking = {}", systemPorperties.trackingProperties.orientationTracking == XR_TRUE, systemPorperties.trackingProperties.positionTracking == XR_TRUE), 1);
	
	configViews = instance.enumerateViewConfigurationViewsToVector(systemId, xr::ViewConfigurationType::PrimaryStereo);
	LOG_INFO("OpenXR", std::format("Number of Views: {}", configViews.size()), 0);

	auto swapChainFormats = session.enumerateSwapchainFormatsToVector();
	swapChainFormat = vk::Format::eB8G8R8A8Srgb;
	bool find = false;
	LOG_INFO("OpenXR", std::format("Searching for Image Format, Desired Format : {}, All available Formats: ", vk::to_string(swapChainFormat)), 0);
	for (auto format : swapChainFormats)
	{
		vk::Format vkFormat = (vk::Format)format;
		if (vkFormat == swapChainFormat)
		{
			find = true;
			LOG_INFO("OpenXR", std::format("{} <= Find.", vk::to_string(vkFormat)), 1);
		}
		else
		{
			LOG_INFO("OpenXR", vk::to_string(vkFormat), 1);
		}
	}
	if (!find)
	{
		swapChainFormat = (vk::Format)swapChainFormats[0];
		LOG_INFO("OpenXR", std::format("Unable to find desired format, falling back to first available format : {}", vk::to_string(swapChainFormat)), 0);
	}

	swapChains.clear();
	swapChainRects.clear();
	swapChainImages.clear();
	for (const auto& view : configViews)
	{
		LOG_STEP("OpenXR", "Creating SwapChain");
		xr::SwapchainCreateInfo swapChainInfo({}, xr::SwapchainUsageFlagBits::Sampled | xr::SwapchainUsageFlagBits::ColorAttachment, (uint64_t)swapChainFormat, view.recommendedSwapchainSampleCount, view.recommendedImageRectWidth, view.recommendedImageRectHeight, 1, 1, 1);
		auto swapChain = session.createSwapchain(swapChainInfo);
		swapChains.push_back(swapChain);
		swapChainRects.push_back({ { 0, 0 }, { static_cast<int32_t>(view.recommendedImageRectWidth), static_cast<int32_t>(view.recommendedImageRectHeight) } });
		swapChainImages.push_back(
			swapChain.enumerateSwapchainImagesToVector<xr::SwapchainImageVulkanKHR>()
		);
		LOG_SUCCESS();
		LOG_INFO("OpenXR", std::format("SwapChainImageType = {}", xr::to_string(swapChainImages[0][0].type)), 0);
		LOG_INFO("OpenXR", std::format("SwapChainImageExtent = ({}, {}, {})", view.recommendedSwapchainSampleCount, view.recommendedImageRectWidth, view.recommendedImageRectHeight), 0);
	}
	graphics.CreateSwapChainImageViews(swapChainImages, swapChainFormat, swapChainRects);
}

bool Noxg::OpenXrInstance::PollEvents()
{
	bool go_on = true;

	xr::Result result = xr::Result::Success;

	while (result == xr::Result::Success)
	{
		eventDataBuffer.type = xr::StructureType::EventDataBuffer;
		result = instance.pollEvent(eventDataBuffer);
		if (result == xr::Result::Success)
		{
			//LOG_INFO("OpenXR", std::format("Event Type : {}", xr::to_string(eventDataBuffer.type)), 0);
			switch (eventDataBuffer.type)
			{
			case xr::StructureType::EventDataInstanceLossPending:
			{
				return false;
				break;
			}
			case xr::StructureType::EventDataSessionStateChanged:
			{
				xr::EventDataSessionStateChanged eventDataSessionStateChanged(*reinterpret_cast<xr::EventDataSessionStateChanged*>(&eventDataBuffer));
				go_on = HandleSessionStateChangedEvent(eventDataSessionStateChanged);
				break;
			}
			default:
				break;
			}

			continue;
		}
	}

	if (result != xr::Result::EventUnavailable)
	{
		throw std::runtime_error(std::format("Unable to read Event. Result : {}", xr::to_string(result)));
	}

	return go_on;
}

bool Noxg::OpenXrInstance::HandleSessionStateChangedEvent(xr::EventDataSessionStateChanged eventDataSessionStateChanged)
{
	auto oldState = sessionState;
	sessionState = eventDataSessionStateChanged.state;
	LOG_INFO("OpenXR", std::format("Session State Changed: {} -> {}", xr::to_string(oldState), xr::to_string(sessionState)), 0);

	switch (sessionState)
	{
	case xr::SessionState::Ready:
	{
		xr::SessionBeginInfo sessionBeginInfo(xr::ViewConfigurationType::PrimaryStereo);
		session.beginSession(sessionBeginInfo);
		sessionRunning = true;
		break;
	}
	case xr::SessionState::Stopping:
	{
		sessionRunning = false;
		session.endSession();
		break;
	}
	case xr::SessionState::Exiting:
	{
		return false;
		break;
	}
	case xr::SessionState::LossPending:
	{
		return false;
		break;
	}
	default:
		break;
	}
	return true;
}

void Noxg::OpenXrInstance::Update()
{
	auto frameState = session.waitFrame({ });
	session.beginFrame({ });

	std::vector<xr::CompositionLayerBaseHeader*> layers = { };
	xr::CompositionLayerProjection layer{ };
	std::vector<xr::CompositionLayerProjectionView> projectionLayerViews;

	if (frameState.shouldRender == XR_TRUE)
	{
		xr::ViewState viewState{ };
		xr::ViewLocateInfo locateInfo(xr::ViewConfigurationType::PrimaryStereo, frameState.predictedDisplayTime, appSpace);
		auto views = session.locateViewsToVector(locateInfo, reinterpret_cast<XrViewState*>(&viewState));
		if ((viewState.viewStateFlags & xr::ViewStateFlagBits::PositionValid) && (viewState.viewStateFlags & xr::ViewStateFlagBits::OrientationValid))
		{
			projectionLayerViews.resize(views.size());
			for (uint32_t i = 0; i < views.size(); ++i)
			{
				auto swapChain = swapChains[i];
				auto imageIndex = swapChain.acquireSwapchainImage({ });
				swapChain.waitSwapchainImage({ xr::Duration::infinite });

				projectionLayerViews[i] = xr::CompositionLayerProjectionView{ views[i].pose, views[i].fov, { swapChain, swapChainRects[i], 0 } };

				graphics.RenderView(projectionLayerViews[i], i, imageIndex, swapChainFormat); // Renderer.

				swapChain.releaseSwapchainImage({ });
			}

			layer.space = appSpace;
			layer.layerFlags = { };
			layer.viewCount = (uint32_t)projectionLayerViews.size();
			layer.views = projectionLayerViews.data();
			layers.push_back(&layer);
		}
	}

	xr::FrameEndInfo endInfo(frameState.predictedDisplayTime, xr::EnvironmentBlendMode::Opaque, layers.size(), layers.data());
	session.endFrame(endInfo);
}

const xr::Instance& Noxg::OpenXrInstance::getInstance() const
{
	return instance;
}

const xr::SystemId& Noxg::OpenXrInstance::getSystemId() const
{
	return systemId;
}
