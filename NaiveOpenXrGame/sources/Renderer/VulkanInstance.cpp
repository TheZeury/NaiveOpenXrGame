#include "mainCommon.h"
#include "VulkanInstance.h"
#include "MeshModel.h"
#include "Utils.h"
#include "XR/OpenXrInstance.h"
#include "Physics/RigidDynamic.h"

Noxg::VulkanInstance::VulkanInstance()
{
	window = nullptr;
	queueFamilyIndex = 0;
	swapChainFormat = vk::Format::eUndefined;
}

Noxg::VulkanInstance::~VulkanInstance()
{
	
}

void Noxg::VulkanInstance::Initialize(rf::XrInstance xrInstance)
{
	auto xrInst = std::static_pointer_cast<OpenXrInstance>(xrInstance.lock());
	openXrInstance = xrInst->getInstance();
	openXrSystemId = xrInst->getSystemId();
	xrInst = nullptr;
	dispather = xr::DispatchLoaderDynamic{ openXrInstance };
#ifdef MIRROR_WINDOW
	CreateWindow();
#endif
	CreateInstance();
	PickPhysicalDevice();
	CreateLogicalDevice();
}

void Noxg::VulkanInstance::CleanUpInstance()
{
	queue.waitIdle();

	LOG_STEP("Vulkan", "Destroying Logical Device");
	device.destroy();
	LOG_SUCCESS();

	LOG_STEP("Vulkan", "Destroying Vulkan Instance");
	instance.destroy();
	LOG_SUCCESS();

#ifdef MIRROR_WINDOW
	glfwDestroyWindow(window);
	glfwTerminate();
#endif
}

void Noxg::VulkanInstance::CleanUpSession()
{
	queue.waitIdle();

#ifdef MIRROR_WINDOW
	instance.destroySurfaceKHR(mirrorSurface);
#endif

	LOG_STEP("Vulkan", "Destroying Models");
	models.clear();
	LOG_SUCCESS();

	LOG_STEP("Vulkan", "Destroying Textures");
	textures.clear();
	LOG_SUCCESS();

	LOG_STEP("Vulkan", "Destroying Descriptor Pool");
	device.destroyDescriptorPool(descriptorPool);
	LOG_SUCCESS();

	LOG_STEP("Vulkan", "Destroying Descriptor Set Layout");
	device.destroyDescriptorSetLayout(textureSetLayout);
	LOG_SUCCESS();

	LOG_STEP("Vulkan", "Destroying Fences");
	for (auto& fence : inFlights)
	{
		device.destroyFence(fence);
	}
	LOG_SUCCESS();

	LOG_STEP("Vulkan", "Destroying Command Buffers");
	device.freeCommandBuffers(commandPool, commandBuffers);
	LOG_SUCCESS();


	LOG_STEP("Vulkan", "Destroying Command Pool");
	device.destroyCommandPool(commandPool);
	LOG_SUCCESS();


	LOG_STEP("Vulkan", "Destroying Frame Buffers");
	for (auto& framebuffers : frameBuffers)
	{
		for (auto& framebuffer : framebuffers)
		{
			device.destroyFramebuffer(framebuffer);
		}
	}
	LOG_SUCCESS();


	LOG_STEP("Vulkan", "Destroying Pipeline");
	device.destroyPipeline(pipeline);
	LOG_SUCCESS();


	LOG_STEP("Vulkan", "Destroying Pipeline Layout");
	device.destroyPipelineLayout(pipelineLayout);
	LOG_SUCCESS();


	LOG_STEP("Vulkan", "Destroying Render Pass");
	device.destroyRenderPass(renderPass);
	LOG_SUCCESS();

	LOG_STEP("Vulkan", "Destroying Depth Resources");
	for (int i = 0; i < depthImages.size(); ++i)
	{
		Utils::destroyImage(depthImages[i], depthImageMemories[i]);
		device.destroyImageView(depthImageViews[i]);
	}
	LOG_SUCCESS();

	LOG_STEP("Vulkan", "Destroying Swap Chain Image Views");
	for (auto& views : swapChainImageViews)
	{
		for (auto& view : views)
		{
			device.destroyImageView(view);
		}
	}
	LOG_SUCCESS();
}

void Noxg::VulkanInstance::CreateWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(1920, 1080, "Hello", nullptr, nullptr);
}

void Noxg::VulkanInstance::CreateInstance()
{
	// Logging Extensions -----------------------------------------------------------
	auto allExtensions = vk::enumerateInstanceExtensionProperties();
	LOG_INFO("Vulkan", std::format("List of total {} Vulkan Instance Extension(s):", allExtensions.size()), 0);
	for (const auto& ext : allExtensions)
	{
		LOG_INFO("Vulkan", ext.extensionName, 1);
	}

	// Creating Instance ------------------------------------------------------------
	if (instance)
	{
		throw std::runtime_error("Can't create a new Vulkan instance when there's already one!");
	}

	std::vector<const char*> extensions;
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	for (uint32_t i = 0; i < glfwExtensionCount; ++i) extensions.push_back(glfwExtensions[i]);
	LOG_INFO("Vulkan", std::format("List of total {} required Vulkan Instance Extension(s):", extensions.size()), 0);
	for (const auto& ext : extensions)
	{
		LOG_INFO("Vulkan", ext, 1);
	}

	// Get VulkanGraphicsRequirements.
	xr::GraphicsRequirementsVulkanKHR requirement = openXrInstance.getVulkanGraphicsRequirements2KHR(openXrSystemId, dispather);
	LOG_INFO("Vulkan", "Vulkan API Version Information: ", 0);
	LOG_INFO("Vulkan", 
		std::format("Lowest supported API version = {}.{}.{} ", 
			requirement.minApiVersionSupported.major(), 
			requirement.minApiVersionSupported.minor(), 
			requirement.minApiVersionSupported.patch()
		),
	1);
	LOG_INFO("Vulkan",
		std::format("Highest verified API version = {}.{}.{} ",
			requirement.maxApiVersionSupported.major(),
			requirement.maxApiVersionSupported.minor(),
			requirement.maxApiVersionSupported.patch()
		),
	1);

	const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};

#define ENABLE_VALIDATION_LAYERS
#if !defined(ENABLE_VALIDATION_LAYERS)
#if !defined(NDEBUG)
#define ENABLE_VALIDATION_LAYERS
#endif
#endif

	LOG_STEP("Vulkan", "Creating Vulkan Instance");
	// App Info.
	vk::ApplicationInfo appInfo("Naive OpenXR Game", 1, "No Engine", 1, VK_API_VERSION_1_3);

	// Instance Info.
#if !defined(ENABLE_VALIDATION_LAYERS)
	vk::InstanceCreateInfo instanceInfo({}, &appInfo, nullptr, extensions);
#else
	vk::InstanceCreateInfo instanceInfo({}, &appInfo, validationLayers, extensions);
#endif
	VkInstanceCreateInfo instInfo = instanceInfo;

	// Xr Vulkan Instance Create Info.
	xr::VulkanInstanceCreateInfoKHR createInfo(openXrSystemId, {}, &vkGetInstanceProcAddr, &instInfo, nullptr);

	// Xr Create Vulkan Instance.
	VkInstance vkInstance;
	VkResult vkResult;
	openXrInstance.createVulkanInstanceKHR(createInfo, &vkInstance, &vkResult, dispather);
	if (vkResult != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan instance.");
	}
	instance = vkInstance;

	LOG_SUCCESS();
}

void Noxg::VulkanInstance::PickPhysicalDevice()
{
	LOG_STEP("Vulkan", "Picking Physical Device");
	xr::VulkanGraphicsDeviceGetInfoKHR getInfo(openXrSystemId, instance);
	physicalDevice = openXrInstance.getVulkanGraphicsDevice2KHR(getInfo, dispather);
	LOG_SUCCESS();
	auto properties = physicalDevice.getProperties();
	LOG_INFO("Vulkan", ("Physical Device Name : " + static_cast<std::string>(properties.deviceName.data())), 0);
	LOG_INFO("Vulkan", std::format("Push Constant Limit : {}", properties.limits.maxPushConstantsSize), 0);
	LOG_INFO("Vulkan", std::format("Max Anisotropy : {}", properties.limits.maxSamplerAnisotropy), 0);
	Utils::maxAnisotrophy = properties.limits.maxSamplerAnisotropy;
	auto features = physicalDevice.getFeatures();
}

void Noxg::VulkanInstance::CreateLogicalDevice()
{
	// Finding for a suitable Queue Family.
	LOG_STEP("Vulkan", "Finding for a suitable Queue Family");
	auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
	queueFamilyIndex = -1;
	for (int i = 0; i < queueFamilyProperties.size(); ++i)
	{
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			queueFamilyIndex = i;
			break;
		}
	}
	if (queueFamilyIndex < 0)
	{
		throw std::runtime_error("Can't find a suitable queue family.");
	}
	LOG_SUCCESS();
	LOG_INFO("Vulkan", std::format("Queue Family Index = {}", queueFamilyIndex), 0);

	std::vector<float> queuePiorities(1, { 0.f });
	vk::DeviceQueueCreateInfo queueInfo({ }, queueFamilyIndex, queuePiorities);

	// Logging Device Extensions
	auto allExtensions = physicalDevice.enumerateDeviceExtensionProperties();
	LOG_INFO("Vulkan", std::format("List of total {} Vulkan Device Extension(s): ", allExtensions.size()), 0);
	for (auto& ext : allExtensions)
	{
		LOG_INFO("Vulkan", std::string(ext.extensionName.data()), 1);
	}

	std::vector<const char*> requiredExtensions(0);
	requiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	LOG_INFO("Vulkan", std::format("List of total {} required Vulkan Device Extension(s): ", requiredExtensions.size()), 0);
	for (auto& ext : requiredExtensions)
	{
		LOG_INFO("Vulkan", ext, 1);
	}

	vk::PhysicalDeviceFeatures deviceFeatures{ };
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	std::vector<vk::DeviceQueueCreateInfo> queueInfos(1, { queueInfo });

	vk::DeviceCreateInfo deviceInfo(
		{ },
		queueInfos,
		{ },
		requiredExtensions,
		&deviceFeatures
	);
	VkDeviceCreateInfo deviInfo = deviceInfo;

	xr::VulkanDeviceCreateInfoKHR createInfo(
		openXrSystemId,
		{},
		&vkGetInstanceProcAddr,
		physicalDevice,
		&deviInfo,
		nullptr
	);

	LOG_STEP("Vulkan", "Creating Vulkan Logical Device");
	VkDevice vkDevice;
	VkResult vkResult;
	openXrInstance.createVulkanDeviceKHR(createInfo, &vkDevice, &vkResult, dispather);
	if (vkResult != VK_SUCCESS)
	{
		throw std::runtime_error("Can't create logical device.");
	}
	device = vkDevice;
	LOG_SUCCESS();

	// Getting Queue.
	LOG_STEP("Vulkan", "Getting Queue");
	queue = device.getQueue(queueFamilyIndex, 0);
	LOG_SUCCESS();
}

void Noxg::VulkanInstance::CreateSwapChainImageViews(std::vector<std::vector<xr::SwapchainImageVulkanKHR>>& swapChainImages, vk::Format format, std::vector<xr::Rect2Di> rects)
{
	swapChainFormat = format;
	swapChainRects = rects;
	LOG_STEP("Vulkan", "Creating Swap Chain Image Views");
	swapChainImageViews.clear();
	for (auto& images : swapChainImages)
	{
		std::vector<vk::ImageView> imageViews; imageViews.clear();
		for (auto& image : images)
		{
			vk::ImageViewCreateInfo createInfo({}, image.image, vk::ImageViewType::e2D, format, { }, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
			imageViews.push_back(device.createImageView(createInfo));
		}
		swapChainImageViews.push_back(imageViews);
	}
	LOG_SUCCESS();
	LOG_INFO("Vulkan", std::format("Swapchain length : {}", swapChainImages[0].size()), 0);

#ifdef MIRROR_WINDOW
	LOG_STEP("Vulkan", "Creating Mirror Window Swap Chain");
#ifndef MIDDLE_EYE_MIRRORING
	glfwSetWindowSize(window, rects[0].extent.width * 1080 / rects[0].extent.height, 1080);
#endif // !MIDDLE_EYE_MIRRORING
	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to create mirror window surface.");
	}
	mirrorSurface = surface;
	if (physicalDevice.getSurfaceSupportKHR(queueFamilyIndex, surface) != VK_TRUE)
	{
		throw std::runtime_error("Presentation not supported.");
	}

	LOG_SUCCESS();
#endif // MIRROR_WINDOW

}

void Noxg::VulkanInstance::InitializeSession()
{
	CreateCommandPool();
	Utils::passInGraphicsInformation(instance, physicalDevice, device, commandPool, queue);
	CreateDepthResources();
	CreateRenderPass();
	CreateDescriptors();
	CreateGraphicsPipeline();
	CreateFrameBuffers();
	AllocateCommandBuffers();
}

void Noxg::VulkanInstance::CreateRenderPass()
{
	vk::AttachmentDescription colorAttachmentDescription({ }, swapChainFormat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eColorAttachmentOptimal);
	vk::AttachmentDescription depthAttachmentDescription({ }, Utils::findDepthFormat(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	std::array<vk::AttachmentDescription, 2> attachments = { colorAttachmentDescription, depthAttachmentDescription };

	vk::AttachmentReference colorAttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
	std::array<vk::AttachmentReference, 1> colorReferences = { colorAttachmentReference };
	vk::AttachmentReference depthAttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass({ }, vk::PipelineBindPoint::eGraphics, { }, colorReferences, { }, &depthAttachmentReference);
	std::array<vk::SubpassDescription, 1> subpasses = { subpass };

	vk::SubpassDependency dependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests, { }, vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
	std::array<vk::SubpassDependency, 1> dependencies = { dependency };

	LOG_STEP("Vulkan", "Creating Render Pass");
	vk::RenderPassCreateInfo createInfo({ }, attachments, subpasses, dependencies);
	renderPass = device.createRenderPass(createInfo);
	LOG_SUCCESS();
}

void Noxg::VulkanInstance::CreateDescriptors()
{
	vk::DescriptorSetLayoutBinding samplerLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, { });
	
	std::array<vk::DescriptorSetLayoutBinding, 1> bindings = { samplerLayoutBinding };
	vk::DescriptorSetLayoutCreateInfo layoutInfo({ }, bindings);
	textureSetLayout = device.createDescriptorSetLayout(layoutInfo);

	std::array<vk::DescriptorPoolSize, 1> poolSizes = {
		vk::DescriptorPoolSize{ vk::DescriptorType::eCombinedImageSampler, static_cast<uint32_t>(swapChainImageViews.size() * 10) },
	};

	vk::DescriptorPoolCreateInfo poolInfo({ }, static_cast<uint32_t>(swapChainImageViews.size() * 10), poolSizes);
	descriptorPool = device.createDescriptorPool(poolInfo);
}

void Noxg::VulkanInstance::CreateGraphicsPipeline()
{
	LOG_STEP("Vulkan", "Loading Shader Modules");
	auto vertShaderCode = readFile("shaders/vert.spv");
	auto fragShaderCode = readFile("shaders/frag.spv");
	vk::ShaderModuleCreateInfo vertInfo({ }, vertShaderCode);
	vk::ShaderModuleCreateInfo fragInfo({ }, fragShaderCode);
	auto vertShaderModule = device.createShaderModule(vertInfo);
	auto fragShaderModule = device.createShaderModule(fragInfo);
	LOG_SUCCESS();

	vk::PipelineShaderStageCreateInfo vertexStageInfo({ }, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main");
	vk::PipelineShaderStageCreateInfo fragmentStageInfo({ }, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main");
	std::vector<vk::PipelineShaderStageCreateInfo> stageInfos = {
		vertexStageInfo,
		fragmentStageInfo,
	};

	std::vector<vk::DynamicState> dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
	};

	vk::PipelineDynamicStateCreateInfo dynamicStateInfo({ }, dynamicStates);

	auto vertexBindingDescriptions = MeshModel::Vertex::getBindingDescriptions();
	auto vertexAttributeDescriptions = MeshModel::Vertex::getAttributeDescriptions();

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{ { }, vertexBindingDescriptions, vertexAttributeDescriptions };

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

	vk::PipelineViewportStateCreateInfo viewportInfo({ }, 1, nullptr, 1, nullptr);

	vk::PipelineRasterizationStateCreateInfo rasterizationInfo({ }, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise, VK_FALSE, { }, { }, { }, 1.f);

	vk::PipelineMultisampleStateCreateInfo multisampleInfo{ };

	vk::PipelineDepthStencilStateCreateInfo depthStencilInfo({ }, VK_TRUE, VK_TRUE, vk::CompareOp::eLess, VK_FALSE, VK_FALSE);

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{ }; colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

	vk::PipelineColorBlendStateCreateInfo colorBlendInfo{ }; colorBlendInfo.attachmentCount = 1; colorBlendInfo.pAttachments = &colorBlendAttachment;

	std::array<vk::DescriptorSetLayout, 1> setLayouts = {
		textureSetLayout
	};

	std::vector<vk::PushConstantRange> pushConstantRanges = {
		{ vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstantData) }
	};

	LOG_STEP("Vulkan", "Creating Pipeline Layout");
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({ }, setLayouts, pushConstantRanges);
	pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);
	LOG_SUCCESS();

	LOG_STEP("Vulkan", "Creating Graphics Pipeline");
	vk::GraphicsPipelineCreateInfo createInfo({ }, stageInfos, &vertexInputInfo, &inputAssemblyInfo, { }, &viewportInfo, &rasterizationInfo, &multisampleInfo, &depthStencilInfo, &colorBlendInfo, &dynamicStateInfo, pipelineLayout, renderPass, 0, { }, -1);
	auto result = device.createGraphicsPipeline({ }, createInfo);
	if (result.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to create graphics pipeline.");
	}
	pipeline = result.value;
	LOG_SUCCESS();

	device.destroyShaderModule(vertShaderModule);
	device.destroyShaderModule(fragShaderModule);
}

void Noxg::VulkanInstance::CreateFrameBuffers()
{
	LOG_STEP("Vulkan", "Creating Swap Chain Frame Buffers");
	frameBuffers.clear();
	int i = 0;
	for (auto& swapChain : swapChainImageViews)
	{
		std::vector<vk::Framebuffer> framebuffers; framebuffers.clear();
		for (auto& view : swapChain)
		{
			std::array<vk::ImageView, 2> attachments = { view, depthImageViews[i] };
			vk::FramebufferCreateInfo createInfo({ }, renderPass, attachments, swapChainRects[i].extent.width, swapChainRects[i].extent.height, 1);
			framebuffers.push_back(device.createFramebuffer(createInfo));
		}
		frameBuffers.push_back(framebuffers);
		++i;
	}
	LOG_SUCCESS();
}

void Noxg::VulkanInstance::CreateCommandPool()
{
	LOG_STEP("Vulkan", "Creating Command Pool");
	vk::CommandPoolCreateInfo createInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndex);
	commandPool = device.createCommandPool(createInfo);
	LOG_SUCCESS();
}

void Noxg::VulkanInstance::CreateDepthResources()
{
	auto depthFormat = Utils::findDepthFormat();
	std::array<uint32_t, 1> queueFamilyIndices = { 0 };
	
	for (int i = 0; i < swapChainImageViews.size(); ++i)
	{
		vk::ImageCreateInfo imageInfo({ }, vk::ImageType::e2D, depthFormat, { static_cast<uint32_t>(swapChainRects[i].extent.width), static_cast<uint32_t>(swapChainRects[i].extent.height), 1}, 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, queueFamilyIndices, vk::ImageLayout::eUndefined);

		auto [depthImage, depthImageMemory] = Utils::CreateImage(imageInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);
		auto depthImageView = Utils::createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
		depthImages.push_back(depthImage);
		depthImageMemories.push_back(depthImageMemory);
		depthImageViews.push_back(depthImageView);
	}
}

void Noxg::VulkanInstance::AllocateCommandBuffers()
{
	LOG_STEP("Vulkan", "Allocating Command Buffers");
	vk::CommandBufferAllocateInfo allocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, static_cast<uint32_t>(swapChainImageViews.size()));
	commandBuffers = device.allocateCommandBuffers(allocateInfo);
	LOG_SUCCESS();

	LOG_STEP("Vulkan", "Creating synchronizers");
	vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);
	inFlights.resize(swapChainImageViews.size());
	for (int i = 0; i < swapChainImageViews.size(); ++i)
	{
		inFlights[i] = device.createFence(fenceInfo);
	}
	LOG_SUCCESS();
}

void Noxg::VulkanInstance::RenderView(xr::CompositionLayerProjectionView projectionView, uint32_t view, uint32_t imageIndex, vk::Format format)
{
	if (device.waitForFences(1, &inFlights[view], VK_TRUE, UINT64_MAX) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to wait for fences.");
	}
	if (device.resetFences(1, &inFlights[view]) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to reset Fences.");
	}

	commandBuffers[view].reset();

	vk::CommandBufferBeginInfo beginInfo{ };
	commandBuffers[view].begin(beginInfo);	// <======= Command Buffer Begin.

	std::array<vk::ClearValue, 2> clearValues = {
		vk::ClearValue{ vk::ClearColorValue{ std::array<float, 4>{ 0.7f, 0.8f, 0.5f, 1.f } } },
		vk::ClearValue{ vk::ClearDepthStencilValue{ 1.f, 0 } },
	};
	vk::RenderPassBeginInfo renderPassInfo(renderPass, frameBuffers[view][imageIndex], { { }, { static_cast<uint32_t>(swapChainRects[view].extent.width), static_cast<uint32_t>(swapChainRects[view].extent.height) } }, clearValues);
	commandBuffers[view].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);		// <======= Render Pass Begin.

	commandBuffers[view].bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);			// <======= Bind Pipeline.

	std::vector<vk::Viewport> viewports = {
		vk::Viewport { 0.f, 0.f, static_cast<float>(swapChainRects[view].extent.width), static_cast<float>(swapChainRects[view].extent.height), 0.f, 1.f }
	};
	commandBuffers[view].setViewport(0, viewports);		// <======== Set Viewports.

	std::vector<vk::Rect2D> scissors = {
		vk::Rect2D { { }, { static_cast<uint32_t>(swapChainRects[view].extent.width), static_cast<uint32_t>(swapChainRects[view].extent.height) } },
	};
	commandBuffers[view].setScissor(0, scissors);		// <======== Set Scissors.

	// Draw something.
	auto pose = projectionView.pose.get();
	XrMatrix4x4f matProjection;		// P
	XrMatrix4x4f_CreateProjectionFov(&matProjection, GRAPHICS_VULKAN, projectionView.fov, DEFAULT_NEAR_Z, INFINITE_FAR_Z);
	XrMatrix4x4f invView;
	XrVector3f identity{ 1.f, 1.f, 1.f };
	XrMatrix4x4f_CreateTranslationRotationScale(&invView, &(pose->position), &(pose->orientation), &identity);
	XrMatrix4x4f matView;		// V
	XrMatrix4x4f_InvertRigidBody(&matView, &invView);
	XrMatrix4x4f matProjectionView;	// PV
	XrMatrix4x4f_Multiply(&matProjectionView, &matProjection, &matView);
	std::vector<PushConstantData> data(1);

	for(auto it = scenes.begin(); it != scenes.end(); )
	{
		auto scene = it->lock();
		if (scene == nullptr)
		{
			it = scenes.erase(it);
			continue;
		}
		++it;

		auto& gameObjects = scene->gameObjects;
		for (auto& obj : gameObjects)
		{
			auto matTransform = obj->transform->getGlobalMatrix();	// M
			data[0].modelMatrix = matTransform;
			XrMatrix4x4f_Multiply(&(data[0].projectionView), &matProjectionView, (XrMatrix4x4f*)&matTransform);	// PVM
			commandBuffers[view].pushConstants<PushConstantData>(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, data);
			for (auto& model : obj->models)
			{
				commandBuffers[view].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, { model->texture->descriptorSet[view] }, { });
				model->bind(commandBuffers[view]);
				model->draw(commandBuffers[view]);
			}
		}
	}
	// End Draw.

	commandBuffers[view].endRenderPass();		// <========= Render Pass End.

	commandBuffers[view].end();		// <========= Command Buffer End.

	vk::SubmitInfo submitInfo{ }; submitInfo.commandBufferCount = 1; submitInfo.pCommandBuffers = &commandBuffers[view];
	queue.submit(submitInfo, inFlights[view]);
}

void Noxg::VulkanInstance::addTexture(hd::Texture texture)
{
	textures.push_back(texture);

	std::vector<vk::DescriptorSetLayout> layouts(swapChainImageViews.size(), textureSetLayout);
	vk::DescriptorSetAllocateInfo allocateInfo(descriptorPool, layouts);
	texture->descriptorSet = device.allocateDescriptorSets(allocateInfo);

	for (size_t i = 0; i < swapChainImageViews.size(); ++i)
	{
		std::array<vk::DescriptorImageInfo, 1> imageInfos = {
			vk::DescriptorImageInfo(texture->textureSampler, texture->textureImageView, vk::ImageLayout::eShaderReadOnlyOptimal)
		};
		std::array<vk::WriteDescriptorSet, 1> descriptorWrites = {
			vk::WriteDescriptorSet(texture->descriptorSet[i], 0, 0, vk::DescriptorType::eCombinedImageSampler, imageInfos)
		};
		device.updateDescriptorSets(descriptorWrites, { });
	}
}

void Noxg::VulkanInstance::addModel(hd::MeshModel model)
{
	models.push_back(model);
}

void Noxg::VulkanInstance::addScene(rf::Scene scene)
{
	scenes.push_back(scene);
}

Noxg::hd::GameObject Noxg::VulkanInstance::loadGameObjectFromFiles(std::string name)
{
	std::string modelDirectory = "models/" + name;
	std::string textureDirectory = modelDirectory + "/textures";
	std::string modelPath = modelDirectory + '/' + name + ".obj";

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	LOG_STEP("Vulkan", "Loading Model File");
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str(), modelDirectory.c_str()))
	{
		throw std::runtime_error(warn + err);
	}
	LOG_SUCCESS();

	std::vector<hd::Texture> texs;
	std::vector<hd::MeshModel> modls;

	LOG_STEP("Vurkan", "Creating Textures");
	for (auto& material : materials)
	{
		texs.push_back(std::make_shared<Texture>(textureDirectory + '/' + material.diffuse_texname));
	}
	LOG_SUCCESS();

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::unordered_map<Vertex, uint32_t> uniqueVertices;

	int shapeCount = 0;
	int materialId = shapes[0].mesh.material_ids[0];	// assume that all faces in a shape have the same material.

	LOG_STEP("Vulkan", "Creating Models");
	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex{ };

			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2],
			};

			vertex.normal = {
				attrib.normals[3 * index.vertex_index + 0],
				attrib.normals[3 * index.vertex_index + 1],
				attrib.normals[3 * index.vertex_index + 2],
			};

			vertex.uv = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.f - attrib.texcoords[2 * index.texcoord_index + 1],
			};

			vertex.color = {
				1.f, 1.f, 1.f, 1.f
			};

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}

		if (++shapeCount >= shapes.size())	// last shape.
		{
			modls.push_back(std::make_shared<MeshModel>(vertices, indices, texs[materialId]));	// make current model.
			// no need to manually clear the containers.
		}
		else if (materialId != shapes[shapeCount].mesh.material_ids[0])	// the next shape has different texture.
		{
			modls.push_back(std::make_shared<MeshModel>(vertices, indices, texs[materialId]));	// make current model.
			vertices.clear();	// clear.
			indices.clear();
			uniqueVertices.clear();
			materialId = shapes[shapeCount].mesh.material_ids[0];	// set material for the next shape.
		}
	}
	LOG_SUCCESS();

	LOG_INFO("Vulkan", std::format("Loaded {} textures and {} models", texs.size(), modls.size()), 0);
	for (auto& texture : texs)
	{
		addTexture(texture);
	}
	for (auto& model : modls)
	{
		addModel(model);
	}

	hd::GameObject obj = std::make_shared<GameObject>();
	obj->models = modls;
	return obj;
}

xr::GraphicsBindingVulkanKHR Noxg::VulkanInstance::getGraphicsBinding()
{
	return xr::GraphicsBindingVulkanKHR(instance, physicalDevice, device, queueFamilyIndex, 0);
}

std::vector<uint32_t> Noxg::VulkanInstance::readFile(const std::string& filepath)
{
	std::ifstream file{ filepath, std::ios::ate | std::ios::binary };

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file: " + filepath);
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<uint32_t> buffer((fileSize / 4) + (bool)(fileSize % 4));

	file.seekg(0);
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

	file.close();
	return buffer;
}