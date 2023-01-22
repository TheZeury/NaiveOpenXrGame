#include "mainCommon.h"
#include "VulkanInstance.h"

Noxg::VulkanInstance::VulkanInstance()
{
}

Noxg::VulkanInstance::~VulkanInstance()
{
	for (auto& fences : inFlights)
	{
		for (auto& fence : fences)
		{
			device.destroyFence(fence);
		}
	}
	device.destroyCommandPool(commandPool);
	for (auto& framebuffers : frameBuffers)
	{
		for (auto& framebuffer : framebuffers)
		{
			device.destroyFramebuffer(framebuffer);
		}
	}
	device.destroyPipeline(pipeline);
	device.destroyPipelineLayout(pipelineLayout);
	device.destroyRenderPass(renderPass);
	for (auto& views : swapChainImageViews)
	{
		for (auto& view : views)
		{
			device.destroyImageView(view);
		}
	}
	device.destroy();
	instance.destroy();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void Noxg::VulkanInstance::Initialize(const xr::Instance& xrInst, const xr::SystemId& xrSysId)
{
	xrInstance = xrInst;
	xrSystemId = xrSysId;
	dispather = xr::DispatchLoaderDynamic{ xrInstance };
	CreateWindow();
	CreateInstance();
	PickPhysicalDevice();
	CreateLogicalDevice();
}

void Noxg::VulkanInstance::CreateWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(800, 600, "Hello", nullptr, nullptr);
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
	for (int i = 0; i < glfwExtensionCount; ++i) extensions.push_back(glfwExtensions[i]);
	LOG_INFO("Vulkan", std::format("List of total {} required Vulkan Instance Extension(s):", extensions.size()), 0);
	for (const auto& ext : extensions)
	{
		LOG_INFO("Vulkan", ext, 1);
	}

	// Get VulkanGraphicsRequirements.
	xr::GraphicsRequirementsVulkanKHR requirement = xrInstance.getVulkanGraphicsRequirements2KHR(xrSystemId, dispather);
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

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	LOG_STEP("Vulkan", "Creating Vulkan Instance");
	// App Info.
	vk::ApplicationInfo appInfo("Naive OpenXR Game", 1, "No Engine", 1, VK_API_VERSION_1_3);

	// Instance Info.
#ifdef NDEBUG
	vk::InstanceCreateInfo instanceInfo({}, &appInfo, nullptr, extensions);
#else
	vk::InstanceCreateInfo instanceInfo({}, &appInfo, validationLayers, extensions);
#endif
	VkInstanceCreateInfo instInfo = instanceInfo;

	// Xr Vulkan Instance Create Info.
	xr::VulkanInstanceCreateInfoKHR createInfo(xrSystemId, {}, &vkGetInstanceProcAddr, &instInfo, nullptr);

	// Xr Create Vulkan Instance.
	VkInstance vkInstance;
	VkResult vkResult;
	xrInstance.createVulkanInstanceKHR(createInfo, &vkInstance, &vkResult, dispather);
	if (vkResult != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan instance.");
	}
	instance = vkInstance;

	LOG_SUCCESS();
}

void Noxg::VulkanInstance::PickPhysicalDevice()
{
	LOG_STEP("Vulkan", "Picking Physical Device");
	xr::VulkanGraphicsDeviceGetInfoKHR getInfo(xrSystemId, instance);
	physicalDevice = xrInstance.getVulkanGraphicsDevice2KHR(getInfo, dispather);
	LOG_SUCCESS();
	auto properties = physicalDevice.getProperties();
	LOG_INFO("Vulkan", ("Physical Device Name : " + static_cast<std::string>(properties.deviceName.data())), 0);
	LOG_INFO("Vulkan", std::format("Push Constant Limit : {}", properties.limits.maxPushConstantsSize), 0);
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

	// Loging Device Extensions
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
		xrSystemId,
		{},
		&vkGetInstanceProcAddr,
		physicalDevice,
		&deviInfo,
		nullptr
	);

	LOG_STEP("Vulkan", "Creating Vulkan Logical Device");
	VkDevice vkDevice;
	VkResult vkResult;
	xrInstance.createVulkanDeviceKHR(createInfo, &vkDevice, &vkResult, dispather);
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
}

void Noxg::VulkanInstance::InitializeSession()
{
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateFrameBuffers();
	CreateCommandPool();
	AllocateCommandBuffers();
}

void Noxg::VulkanInstance::CreateRenderPass()
{
	vk::AttachmentDescription colorAttachmentDescription({ }, swapChainFormat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eColorAttachmentOptimal);
	std::vector<vk::AttachmentDescription> attachments = { colorAttachmentDescription };

	vk::AttachmentReference colorAttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
	std::vector<vk::AttachmentReference> colorReferences = { colorAttachmentReference };

	vk::SubpassDescription subpass({ }, vk::PipelineBindPoint::eGraphics, { }, colorReferences);
	std::vector<vk::SubpassDescription> subpasses = { subpass };

	vk::SubpassDependency dependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput, { }, vk::AccessFlagBits::eColorAttachmentWrite);
	std::vector<vk::SubpassDependency> dependencies = { dependency };

	LOG_STEP("Vulkan", "Creating Render Pass");
	vk::RenderPassCreateInfo createInfo({ }, attachments, subpasses, dependencies);
	renderPass = device.createRenderPass(createInfo);
	LOG_SUCCESS();
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

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{ };

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

	vk::PipelineViewportStateCreateInfo viewportInfo({ }, 1, nullptr, 1, nullptr);

	vk::PipelineRasterizationStateCreateInfo rasterizationInfo({ }, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise, VK_FALSE, { }, { }, { }, 1.f);

	vk::PipelineMultisampleStateCreateInfo multisampleInfo{ };

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{ }; colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

	vk::PipelineColorBlendStateCreateInfo colorBlendInfo{ }; colorBlendInfo.attachmentCount = 1; colorBlendInfo.pAttachments = &colorBlendAttachment;

	std::vector<vk::PushConstantRange> pushConstantRanges = {
		{ vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstantData) }
	};

	LOG_STEP("Vulkan", "Creating Pipeline Layout");
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({ }, { }, pushConstantRanges);
	pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);
	LOG_SUCCESS();

	LOG_STEP("Vulkan", "Creating Graphics Pipeline");
	vk::GraphicsPipelineCreateInfo createInfo({ }, stageInfos, &vertexInputInfo, &inputAssemblyInfo, { }, &viewportInfo, &rasterizationInfo, &multisampleInfo, { }, &colorBlendInfo, &dynamicStateInfo, pipelineLayout, renderPass, 0, { }, -1);
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
	for (auto& swapChain : swapChainImageViews)
	{
		std::vector<vk::Framebuffer> framebuffers; framebuffers.clear();
		for (auto& view : swapChain)
		{
			std::vector<vk::ImageView> attachments = { view };
			vk::FramebufferCreateInfo createInfo({}, renderPass, attachments, swapChainRects[0].extent.width, swapChainRects[0].extent.height, 1);
			framebuffers.push_back(device.createFramebuffer(createInfo));
		}
		frameBuffers.push_back(framebuffers);
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

void Noxg::VulkanInstance::AllocateCommandBuffers()
{
	LOG_STEP("Vulkan", "Allocating Command Buffers");
	vk::CommandBufferAllocateInfo allocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, swapChainImageViews.size());
	commandBuffers = device.allocateCommandBuffers(allocateInfo);
	LOG_SUCCESS();

	LOG_STEP("Vulkan", "Creating synchronizers");
	vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);
	inFlights.resize(swapChainImageViews.size());
	for (int i = 0; i < swapChainImageViews.size(); ++i)
	{
		inFlights[i].resize(MAX_FRAMES_IN_FLIGHT);
		for (int j = 0; j < MAX_FRAMES_IN_FLIGHT; ++j)
		{
			inFlights[i][j] = device.createFence(fenceInfo);
		}
	}
	LOG_SUCCESS();
}

void Noxg::VulkanInstance::RenderView(xr::CompositionLayerProjectionView projectionView, uint32_t view, uint32_t imageIndex, vk::Format format)
{
	device.waitForFences(1, &inFlights[view][0], VK_TRUE, UINT64_MAX);
	device.resetFences(1, &inFlights[view][0]);

	commandBuffers[view].reset();

	vk::CommandBufferBeginInfo beginInfo{ };
	commandBuffers[view].begin(beginInfo);	// <======= Command Buffer Begin.

	std::vector<vk::ClearValue> clearValues = {
		/*{ { { 0.2f, 0.8f, 0.1f, 1.f } } }*/
		{ { { 0.7f, 0.8f, 0.5f, 1.f } } }
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
	XrMatrix4x4f matProjection;
	XrMatrix4x4f_CreateProjectionFov(&matProjection, GRAPHICS_VULKAN, projectionView.fov, 0.05f, 100.f);
	XrMatrix4x4f invView;
	XrVector3f identity{ 1.f, 1.f, 1.f };
	XrMatrix4x4f_CreateTranslationRotationScale(&invView, &(pose->position), &(pose->orientation), &identity);
	XrMatrix4x4f matView;
	XrMatrix4x4f_InvertRigidBody(&matView, &invView);
	std::vector<PushConstantData> data(1);
	XrMatrix4x4f_Multiply(&(data[0].projectionView), &matProjection, &matView);
	commandBuffers[view].pushConstants<PushConstantData>(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, data);
	commandBuffers[view].draw(3, 1, 0, 0);
	// End Draw.

	commandBuffers[view].endRenderPass();		// <========= Render Pass End.

	commandBuffers[view].end();		// <========= Command Buffer End.

	vk::SubmitInfo submitInfo{ }; submitInfo.commandBufferCount = 1; submitInfo.pCommandBuffers = &commandBuffers[view];
	queue.submit(submitInfo, inFlights[view][0]);
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