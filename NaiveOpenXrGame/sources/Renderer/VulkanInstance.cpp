#include "mainCommon.h"
#include "VulkanInstance.h"
#include "MeshModel.h"
#include "Material.h"
#include "Utils.h"
#include "XR/OpenXrInstance.h"
#include "Physics/RigidDynamic.h"
#include "TextModel.h"
#include "UIElement.h"

import NoxgMath;

void GlfwErrorCallback(int code, const char* description)
{
	LOG_ERRO(std::format("GLFW ERROR [{}]:\t{}", code, description));
}

Noxg::VulkanInstance::VulkanInstance()
{
	queueFamilyIndex = 0;
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
	dispatcher = xr::DispatchLoaderDynamic{ openXrInstance };
#ifdef MIRROR_WINDOW
	CreateWindow();
#endif
	CreateInstance();
	PickPhysicalDevice();
	CreateLogicalDevice();
	Utils::vkDevice = device;
	Utils::vkPhysicalDevice = physicalDevice;
	CreateCommandPool();
	Utils::passInGraphicsInformation(instance, physicalDevice, device, commandPool, queue);
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
	device.destroySemaphore(mirrorImageAvailableSemaphore);
	device.destroySwapchainKHR(mirrorVkSwapchain);
	mirrorSwapchain = nullptr;
	instance.destroySurfaceKHR(mirrorSurface);
#endif

	//preservedModels.clear();

	Texture::empty = nullptr;

	LOG_STEP("Vulkan", "Destroying Descriptor Pool");
	device.destroyDescriptorPool(descriptorPool);
	LOG_SUCCESS();

	LOG_STEP("Vulkan", "Destroying Descriptor Set Layout");
	device.destroyDescriptorSetLayout(Material::materialSetLayout);
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

	LOG_STEP("Vulkan", "Destroying Swap Chains");
	swapChains.clear();
	LOG_SUCCESS();

	LOG_STEP("Vulkan", "Destroying Pipeline");
	device.destroyPipeline(pipelines.textPipeline);
	device.destroyPipeline(pipelines.meshPipeline);
	device.destroyPipeline(pipelines.wireframePipeline);
	LOG_SUCCESS();

	LOG_STEP("Vulkan", "Destroying Pipeline Layout");
	device.destroyPipelineLayout(pipelineLayouts.worldPipelineLayout);
	LOG_SUCCESS();


	LOG_STEP("Vulkan", "Destroying Render Pass");
	device.destroyRenderPass(renderPass);
	LOG_SUCCESS();
}

#ifdef MIRROR_WINDOW
void Noxg::VulkanInstance::CreateWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(1920, 1080, "Naive OpenXR Game", nullptr, nullptr);
	glfwSetErrorCallback(GlfwErrorCallback);
}
#endif

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
#ifdef MIRROR_WINDOW
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	for (uint32_t i = 0; i < glfwExtensionCount; ++i) extensions.push_back(glfwExtensions[i]);
#endif
	LOG_INFO("Vulkan", std::format("List of total {} required Vulkan Instance Extension(s):", extensions.size()), 0);
	for (const auto& ext : extensions)
	{
		LOG_INFO("Vulkan", ext, 1);
	}

	// Get VulkanGraphicsRequirements.
	xr::GraphicsRequirementsVulkanKHR requirement = openXrInstance.getVulkanGraphicsRequirements2KHR(openXrSystemId, dispatcher);
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
	openXrInstance.createVulkanInstanceKHR(createInfo, &vkInstance, &vkResult, dispatcher);
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
	physicalDevice = openXrInstance.getVulkanGraphicsDevice2KHR(getInfo, dispatcher);
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
		if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)/* && physicalDevice.getSurfaceSupportKHR(i, mirrorSurface) // we'll check it later. */)
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
	openXrInstance.createVulkanDeviceKHR(createInfo, &vkDevice, &vkResult, dispatcher);
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
	CreateRenderPass(format);
	LOG_STEP("Vulkan", "Creating Swap Chains");

	swapChains.clear();
	std::vector<vk::Image> images;
	for (int i = 0; i < swapChainImages.size(); ++i)
	{
		images.clear();
		for (auto& image : swapChainImages[i])
		{
			images.push_back(image.image);
		}
		swapChains.push_back(std::make_shared<SwapChain>(device, renderPass, images, format, vk::Rect2D{ { rects[i].offset.x, rects[i].offset.y }, { static_cast<uint32_t>(rects[i].extent.width), static_cast<uint32_t>(rects[i].extent.height) } }));
	}
	LOG_SUCCESS();
	LOG_INFO("Vulkan", std::format("Swapchain length : {}", swapChainImages[0].size()), 0);

#ifdef MIRROR_WINDOW
	LOG_STEP("Vulkan", "Creating Mirror Window Swap Chain");

	glfwSetWindowSize(window, rects[0].extent.width * mirrorImageHeight / rects[0].extent.height, mirrorImageHeight);
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

	vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(mirrorSurface);
	auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(mirrorSurface);
	auto presentModes = physicalDevice.getSurfacePresentModesKHR(mirrorSurface);
	if (surfaceFormats.empty() || presentModes.empty())
	{
		throw std::runtime_error("Swapchain not supported.");
	}
	vk::SurfaceFormatKHR choosedMirrorSurfaceFormat; bool surfaceFormatFound = false;
	for (const auto& surfaceFormat : surfaceFormats)
	{
		if (surfaceFormat.format == format && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear)
		{
			choosedMirrorSurfaceFormat = surfaceFormat;
			surfaceFormatFound = true;
			break;
		}
	}
	if (!surfaceFormatFound)
	{
		throw std::runtime_error(std::format("Can's find desired surface format({}, {}) for mirror window.", vk::to_string(format), vk::to_string(vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear)));
	}
	vk::PresentModeKHR choosedPresentMode; bool presentModefound = false;
	for (const auto& presentMode : presentModes)
	{
		if (presentMode == vk::PresentModeKHR::eMailbox)
		{
			choosedPresentMode = presentMode;
			presentModefound = true;
			break;
		}
	}
	if (!presentModefound) for (const auto& presentMode : presentModes)
	{
		if (presentMode == vk::PresentModeKHR::eImmediate)
		{
			choosedPresentMode = presentMode;
			presentModefound = true;
			break;
		}
	}
	if (!presentModefound)
	{
		throw std::runtime_error(std::format("Can't find desired present mode({} or {}) for mirror window", vk::to_string(vk::PresentModeKHR::eMailbox), vk::to_string(vk::PresentModeKHR::eImmediate)));
	}
	vk::Extent2D mirrorWindowExtent;
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		mirrorWindowExtent = capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		mirrorWindowExtent.width = std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		mirrorWindowExtent.height = std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	}
	std::cout << "Extent: [" << mirrorWindowExtent.width << ", " << mirrorWindowExtent.height << "]\t";
	uint32_t imageCount = capabilities.maxImageCount ? std::min(capabilities.maxImageCount, capabilities.minImageCount + 1) : (capabilities.minImageCount + 1);
	
	vk::SwapchainCreateInfoKHR mirrorSwapchainCreateInfo({ }, mirrorSurface, imageCount, choosedMirrorSurfaceFormat.format, choosedMirrorSurfaceFormat.colorSpace, mirrorWindowExtent,
		1, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, { },
		capabilities.currentTransform, vk::CompositeAlphaFlagBitsKHR::eOpaque, choosedPresentMode, VK_FALSE, { });
	mirrorVkSwapchain = device.createSwapchainKHR(mirrorSwapchainCreateInfo);

	auto mirrorSwapchainImages = device.getSwapchainImagesKHR(mirrorVkSwapchain);
	mirrorSwapchain = std::make_shared<SwapChain>(device, nullptr, mirrorSwapchainImages, choosedMirrorSurfaceFormat.format, vk::Rect2D{ { }, mirrorWindowExtent });

	vk::SemaphoreCreateInfo semaphoreInfo{ };
	mirrorImageAvailableSemaphore = device.createSemaphore(semaphoreInfo);

	LOG_SUCCESS();
#endif // MIRROR_WINDOW

}

void Noxg::VulkanInstance::InitializeSession()
{
	CreateDescriptors();
	unsigned char pixels[] = { 0, 0, 0, 0 };
	Texture::empty = std::make_shared<Texture>(pixels, 1, 1, 4);
	CreateGraphicsPipelines();
	AllocateCommandBuffers();
}

void Noxg::VulkanInstance::CreateRenderPass(vk::Format format)
{
	vk::AttachmentDescription colorAttachmentDescription({ }, format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eColorAttachmentOptimal);
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
	Material::materialSetLayout = Material::getDescriptorSetLayout();
	CharacterBitmap::bitmapSetLayout = CharacterBitmap::getDescriptorSetLayout();

	std::array<vk::DescriptorPoolSize, 2> poolSizes = {
		vk::DescriptorPoolSize{ vk::DescriptorType::eUniformBuffer, 10 },
		vk::DescriptorPoolSize{ vk::DescriptorType::eCombinedImageSampler, 30 },
	};

	vk::DescriptorPoolCreateInfo poolInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 100, poolSizes);	// TODO performance concern.
	descriptorPool = device.createDescriptorPool(poolInfo);
	Material::descriptorPool = descriptorPool;
	CharacterBitmap::descriptorPool = descriptorPool;
}

void Noxg::VulkanInstance::CreateGraphicsPipelines()
{
	LOG_STEP("Vulkan", "Loading Shader Modules");
	auto meshVertShaderCode = readFile("shaders/mesh.vert.spv");
	auto meshFragShaderCode = readFile("shaders/mesh.frag.spv");
	vk::ShaderModuleCreateInfo meshVertInfo({ }, meshVertShaderCode);
	vk::ShaderModuleCreateInfo meshFragInfo({ }, meshFragShaderCode);
	auto meshVertShaderModule = device.createShaderModule(meshVertInfo);
	auto meshFragShaderModule = device.createShaderModule(meshFragInfo);

	auto textVertShaderCode = readFile("shaders/text.vert.spv");
	auto textFragShaderCode = readFile("shaders/text.frag.spv");
	vk::ShaderModuleCreateInfo textVertInfo({ }, textVertShaderCode);
	vk::ShaderModuleCreateInfo textFragInfo({ }, textFragShaderCode);
	auto textVertShaderModule = device.createShaderModule(textVertInfo);
	auto textFragShaderModule = device.createShaderModule(textFragInfo);

	auto uiVertShaderCode = readFile("shaders/ui.vert.spv");
	auto uiFragShaderCode = readFile("shaders/ui.frag.spv");
	vk::ShaderModuleCreateInfo uiVertInfo({ }, uiVertShaderCode);
	vk::ShaderModuleCreateInfo uiFragInfo({ }, uiFragShaderCode);
	auto uiVertShaderModule = device.createShaderModule(uiVertInfo);
	auto uiFragShaderModule = device.createShaderModule(uiFragInfo);
	LOG_SUCCESS();

	vk::PipelineShaderStageCreateInfo meshVertexStageInfo({ }, vk::ShaderStageFlagBits::eVertex, meshVertShaderModule, "main");
	vk::PipelineShaderStageCreateInfo meshFragmentStageInfo({ }, vk::ShaderStageFlagBits::eFragment, meshFragShaderModule, "main");
	std::vector<vk::PipelineShaderStageCreateInfo> meshStageInfos = {
		meshVertexStageInfo,
		meshFragmentStageInfo,
	};

	vk::PipelineShaderStageCreateInfo textVertexStageInfo({ }, vk::ShaderStageFlagBits::eVertex, textVertShaderModule, "main");
	vk::PipelineShaderStageCreateInfo textFragmentStageInfo({ }, vk::ShaderStageFlagBits::eFragment, textFragShaderModule, "main");
	std::vector<vk::PipelineShaderStageCreateInfo> textStageInfos = {
		textVertexStageInfo,
		textFragmentStageInfo,
	};

	vk::PipelineShaderStageCreateInfo uiVertexStageInfo({ }, vk::ShaderStageFlagBits::eVertex, uiVertShaderModule, "main");
	vk::PipelineShaderStageCreateInfo uiFragmentStageInfo({ }, vk::ShaderStageFlagBits::eFragment, uiFragShaderModule, "main");
	std::vector<vk::PipelineShaderStageCreateInfo> uiStageInfos = {
		uiVertexStageInfo,
		uiFragmentStageInfo,
	};

	std::vector<vk::DynamicState> dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
	};

	vk::PipelineDynamicStateCreateInfo dynamicStateInfo({ }, dynamicStates);

	auto meshVertexBindingDescriptions = MeshModel::Vertex::getBindingDescriptions();
	auto meshVertexAttributeDescriptions = MeshModel::Vertex::getAttributeDescriptions();
	vk::PipelineVertexInputStateCreateInfo meshVertexInputInfo{ { }, meshVertexBindingDescriptions, meshVertexAttributeDescriptions };

	auto textVertexBindingDescriptions = TextModel::TextVertex::getBindingDescriptions();
	auto textVertexAttributeDescriptions = TextModel::TextVertex::getAttributeDescriptions();
	vk::PipelineVertexInputStateCreateInfo textVertexInputInfo{ { }, textVertexBindingDescriptions, textVertexAttributeDescriptions };

	auto uiVertexBindingDescriptions = UIElement::UIVertex::getBindingDescriptions();
	auto uiVertexAttributeDescriptions = UIElement::UIVertex::getAttributeDescriptions();
	vk::PipelineVertexInputStateCreateInfo uiVertexInputInfo{ { }, uiVertexBindingDescriptions, uiVertexAttributeDescriptions };

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

	vk::PipelineViewportStateCreateInfo viewportInfo({ }, 1, nullptr, 1, nullptr);

	vk::PipelineRasterizationStateCreateInfo fillRasterizationInfo({ }, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, VK_FALSE, { }, { }, { }, 1.f);

	vk::PipelineRasterizationStateCreateInfo wireRasterizationInfo({ }, VK_FALSE, VK_FALSE, vk::PolygonMode::eLine, vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise, VK_FALSE, { }, { }, { }, 1.f);

	vk::PipelineMultisampleStateCreateInfo multisampleInfo{ };

	vk::PipelineDepthStencilStateCreateInfo depthStencilInfo({ }, VK_TRUE, VK_TRUE, vk::CompareOp::eLess, VK_FALSE, VK_FALSE);

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{ }; colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

	vk::PipelineColorBlendStateCreateInfo colorBlendInfo{ }; colorBlendInfo.attachmentCount = 1; colorBlendInfo.pAttachments = &colorBlendAttachment;

	std::array<vk::DescriptorSetLayout, 2> setLayouts = {
		Material::materialSetLayout,
		CharacterBitmap::bitmapSetLayout,
	};

	std::vector<vk::PushConstantRange> pushConstantRanges = {
		{ vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstantData) }
	};

	LOG_STEP("Vulkan", "Creating Pipeline Layout");
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({ }, setLayouts, pushConstantRanges);
	pipelineLayouts.worldPipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);
	LOG_SUCCESS();

	LOG_STEP("Vulkan", "Creating Graphics Pipelines");
	vk::GraphicsPipelineCreateInfo textPipelineCreateInfo({ }, textStageInfos, &textVertexInputInfo, &inputAssemblyInfo, { }, &viewportInfo, &fillRasterizationInfo, &multisampleInfo, &depthStencilInfo, &colorBlendInfo, &dynamicStateInfo, pipelineLayouts.worldPipelineLayout, renderPass, 0, { }, -1);
	auto result = device.createGraphicsPipeline({ }, textPipelineCreateInfo);
	if (result.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to create text graphics pipeline.");
	}
	pipelines.textPipeline = result.value;
	
	vk::GraphicsPipelineCreateInfo meshPipelineCreateInfo({ }, meshStageInfos, &meshVertexInputInfo, &inputAssemblyInfo, { }, &viewportInfo, &fillRasterizationInfo, &multisampleInfo, &depthStencilInfo, &colorBlendInfo, &dynamicStateInfo, pipelineLayouts.worldPipelineLayout, renderPass, 0, { }, -1);
	result = device.createGraphicsPipeline({ }, meshPipelineCreateInfo);
	if (result.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to create mesh graphics pipeline.");
	}
	pipelines.meshPipeline = result.value;

	vk::GraphicsPipelineCreateInfo wireframePipelineCreateInfo({ }, meshStageInfos, &meshVertexInputInfo, &inputAssemblyInfo, { }, &viewportInfo, &wireRasterizationInfo, &multisampleInfo, &depthStencilInfo, &colorBlendInfo, &dynamicStateInfo, pipelineLayouts.worldPipelineLayout, renderPass, 0, { }, -1);
	result = device.createGraphicsPipeline({ }, wireframePipelineCreateInfo);
	if (result.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to create mesh graphics pipeline.");
	}
	pipelines.wireframePipeline = result.value;

	vk::GraphicsPipelineCreateInfo uiPipelineCreateInfo({ }, uiStageInfos, &uiVertexInputInfo, &inputAssemblyInfo, { }, &viewportInfo, &fillRasterizationInfo, &multisampleInfo, &depthStencilInfo, &colorBlendInfo, &dynamicStateInfo, pipelineLayouts.worldPipelineLayout, renderPass, 0, { }, -1);
	result = device.createGraphicsPipeline({ }, uiPipelineCreateInfo);
	if (result.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to create UI graphics pipeline.");
	}
	pipelines.uiPipeline = result.value;
	LOG_SUCCESS();

	device.destroyShaderModule(meshVertShaderModule);
	device.destroyShaderModule(meshFragShaderModule);
	device.destroyShaderModule(textVertShaderModule);
	device.destroyShaderModule(textFragShaderModule);
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
	vk::CommandBufferAllocateInfo allocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, static_cast<uint32_t>(swapChains.size()));
	commandBuffers = device.allocateCommandBuffers(allocateInfo);
	LOG_SUCCESS();

	//preservedModels.clear();
	//preservedModels.resize(swapChains.size());

	LOG_STEP("Vulkan", "Creating synchronizers");
	vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);
	inFlights.resize(swapChains.size());
	for (int i = 0; i < swapChains.size(); ++i)
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

#ifdef MIRROR_WINDOW
	bool iconified = glfwGetWindowAttrib(window, GLFW_ICONIFIED);
	uint32_t mirrorImageIndex = (view == mirrorView && !iconified) ? device.acquireNextImageKHR(mirrorVkSwapchain, UINT64_MAX, mirrorImageAvailableSemaphore, {}).value : UINT32_MAX;
#endif

	//preservedModels[view].clear();

	commandBuffers[view].reset();

	vk::CommandBufferBeginInfo beginInfo{ };
	commandBuffers[view].begin(beginInfo);	// <======= Command Buffer Begin.

	commandBuffers[view].beginRenderPass(swapChains[view]->getRenderPassBeginInfo(imageIndex), vk::SubpassContents::eInline);		// <======= Render Pass Begin.

	const auto pose = projectionView.pose.get();
	XrMatrix4x4f matProjection;		// P
	XrMatrix4x4f_CreateProjectionFov(&matProjection, GRAPHICS_VULKAN, projectionView.fov, DEFAULT_NEAR_Z, INFINITE_FAR_Z);

	std::vector<hd::Scene> scenes;
	bool haveText = false;
	bool haveMesh = false;
	bool haveUI = false;
	bool haveDebug = false;

	XrMatrix4x4f noCameraInvView; 
	XrVector3f identity{ 1.f, 1.f, 1.f };
	XrMatrix4x4f_CreateTranslationRotationScale(&noCameraInvView, &(pose->position), &(pose->orientation), &identity);
	XrMatrix4x4f noCameraView;
	XrMatrix4x4f_InvertRigidBody(&noCameraView, &noCameraInvView);

	for (auto it = this->scenes.begin(); it != this->scenes.end(); )
	{
		auto scene = it->lock();
		if (scene == nullptr)
		{
			it = this->scenes.erase(it);
			continue;
		}
		++it;

		scenes.push_back(scene);

		haveText |= !(scene->debugType == DebugType::eDebugOnly || scene->texts.empty());
		haveMesh |= !(scene->debugType == DebugType::eDebugOnly || scene->models.empty());
		haveUI |= !(scene->uiElements.empty());	// No `scene->onlyDebug` like above because we till want to have UI even in debug view.
		haveDebug |= debugOn(scene->debugType) && !(scene->debugScene == nullptr || scene->debugScene->models.empty());

		if (!scene->cameraTransform.expired())
		{
			scene->cameraTransform.lock()->setLocalPosition(*(glm::vec3*)(&(pose->position)));
			scene->cameraTransform.lock()->setLocalRotation(*(glm::quat*)(&(pose->orientation)));
		}
	}
	
	if(haveText)
	{
		commandBuffers[view].bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines.textPipeline);			// <======= Bind Text Pipeline.

		commandBuffers[view].setViewport(0, 1, swapChains[view]->getViewport());		// <======== Set Viewports.

		commandBuffers[view].setScissor(0, 1, swapChains[view]->getScissor());		// <======== Set Scissors.

		// Draw something.
		for (auto scene : scenes)
		{
			if (scene->debugType == DebugType::eDebugOnly || scene->texts.empty()) continue;

			XrMatrix4x4f matView;		// V
			if (scene->cameraTransform.expired())
			{
				matView = noCameraView;
			}
			else
			{
				XrMatrix4x4f invView = cnv<XrMatrix4x4f>(scene->cameraTransform.lock()->getGlobalMatrix());
				XrMatrix4x4f_InvertRigidBody(&matView, &invView);
			}

			XrMatrix4x4f matProjectionView;	// PV
			XrMatrix4x4f_Multiply(&matProjectionView, &matProjection, &matView);
			std::vector<PushConstantData> data(1);

			std::pair<hd::TextModel, hd::ITransform> last = { nullptr, nullptr };
			for (auto& pair : scene->texts)
			{
				auto text = pair.first;
				if (text != last.first) 
				{
					text->bind(commandBuffers[view]);
					commandBuffers[view].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayouts.worldPipelineLayout, 0, { text->material->descriptorSet }, { });
					commandBuffers[view].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayouts.worldPipelineLayout, 1, { text->bitmap->descriptorSet }, { });
					last.first = text;
				}

				auto transform = pair.second;
				if (transform != last.second)
				{
					auto matTransform = pair.second->getGlobalMatrix();	// M
					data[0].modelMatrix = matTransform;
					XrMatrix4x4f_Multiply(&(data[0].projectionView), &matProjectionView, (XrMatrix4x4f*)&matTransform);	// PVM
					commandBuffers[view].pushConstants<PushConstantData>(pipelineLayouts.worldPipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, data);
				}

				text->draw(commandBuffers[view]);
			}
		}
		// End Draw.
	}

	if (haveMesh)
	{
		commandBuffers[view].bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines.meshPipeline);			// <======= Bind Mesh Pipeline.

		commandBuffers[view].setViewport(0, 1, swapChains[view]->getViewport());		// <======== Set Viewports.

		commandBuffers[view].setScissor(0, 1, swapChains[view]->getScissor());		// <======== Set Scissors.

		// Draw something.
		for (auto scene : scenes)
		{
			if (scene->debugType == DebugType::eDebugOnly || scene->models.empty()) continue;

			XrMatrix4x4f matView;		// V
			if (scene->cameraTransform.expired())
			{
				matView = noCameraView;
			}
			else
			{
				XrMatrix4x4f invView = cnv<XrMatrix4x4f>(scene->cameraTransform.lock()->getGlobalMatrix());
				XrMatrix4x4f_InvertRigidBody(&matView, &invView);
			}

			XrMatrix4x4f matProjectionView;	// PV
			XrMatrix4x4f_Multiply(&matProjectionView, &matProjection, &matView);
			std::vector<PushConstantData> data(1);

			std::pair<hd::MeshModel, hd::ITransform> last = { nullptr, nullptr };
			for (auto& pair : scene->models)
			{
				auto model = pair.first;
				if (model != last.first)
				{
					model->bind(commandBuffers[view]);
					commandBuffers[view].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayouts.worldPipelineLayout, 0, { model->material->descriptorSet }, { });
					last.first = model;
				}

				auto transform = pair.second;
				if (transform != last.second)
				{
					auto matTransform = pair.second->getGlobalMatrix();	// M
					data[0].modelMatrix = matTransform;
					XrMatrix4x4f_Multiply(&(data[0].projectionView), &matProjectionView, (XrMatrix4x4f*)&matTransform);	// PVM
					commandBuffers[view].pushConstants<PushConstantData>(pipelineLayouts.worldPipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, data);
				}

				model->draw(commandBuffers[view]);
			}
		}
		// End Draw.
	}

	if (haveUI)
	{
		commandBuffers[view].bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines.uiPipeline);			// <======= Bind UI Pipeline.

		commandBuffers[view].setViewport(0, 1, swapChains[view]->getViewport());		// <======== Set Viewports.

		commandBuffers[view].setScissor(0, 1, swapChains[view]->getScissor());		// <======== Set Scissors.

		// Draw something.
		for (auto scene : scenes)
		{
			if (scene->uiElements.empty()) continue;	// No `scene->onlyDebug` like above because we till want to have UI even in debug view.

			XrMatrix4x4f matView;		// V
			if (scene->cameraTransform.expired())
			{
				matView = noCameraView;
			}
			else
			{
				XrMatrix4x4f invView = cnv<XrMatrix4x4f>(scene->cameraTransform.lock()->getGlobalMatrix());
				XrMatrix4x4f_InvertRigidBody(&matView, &invView);
			}

			XrMatrix4x4f matProjectionView;	// PV
			XrMatrix4x4f_Multiply(&matProjectionView, &matProjection, &matView);
			std::vector<PushConstantData> data(1);

			std::pair<hd::UIElement, hd::ITransform> last = { nullptr, nullptr };
			for (auto& pair : scene->uiElements)
			{
				auto element = pair.first;
				if (element != last.first)
				{
					element->bind(commandBuffers[view]);
					commandBuffers[view].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayouts.worldPipelineLayout, 1, { element->bitmap->descriptorSet }, { });
					last.first = element;
				}

				auto transform = pair.second;
				if (transform != last.second)
				{
					auto matTransform = pair.second->getGlobalMatrix();	// M
					data[0].modelMatrix = matTransform;
					XrMatrix4x4f_Multiply(&(data[0].projectionView), &matProjectionView, (XrMatrix4x4f*)&matTransform);	// PVM
					commandBuffers[view].pushConstants<PushConstantData>(pipelineLayouts.worldPipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, data);
				}

				element->draw(commandBuffers[view]);
			}
		}
		// End Draw.
	}

	if(haveDebug)
	{
		commandBuffers[view].bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines.wireframePipeline);			// <======= Bind Wireframe Pipeline.

		commandBuffers[view].setViewport(0, 1, swapChains[view]->getViewport());		// <======== Set Viewports.

		commandBuffers[view].setScissor(0, 1, swapChains[view]->getScissor());		// <======== Set Scissors.

		// Draw something.
		for (auto scene : scenes)
		{
			if (!debugOn(scene->debugType)) continue;
			scene = scene->debugScene;
			if (scene == nullptr || scene->models.empty()) continue;

			XrMatrix4x4f matView;		// V
			if (scene->cameraTransform.expired())
			{
				matView = noCameraView;
			}
			else
			{
				XrMatrix4x4f invView = cnv<XrMatrix4x4f>(scene->cameraTransform.lock()->getGlobalMatrix());
				XrMatrix4x4f_InvertRigidBody(&matView, &invView);
			}

			XrMatrix4x4f matProjectionView;	// PV
			XrMatrix4x4f_Multiply(&matProjectionView, &matProjection, &matView);
			std::vector<PushConstantData> data(1);

			std::pair<hd::MeshModel, hd::ITransform> last = { nullptr, nullptr };
			for (auto& pair : scene->models)
			{
				auto model = pair.first;
				if (model != last.first)
				{
					model->bind(commandBuffers[view]);
					commandBuffers[view].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayouts.worldPipelineLayout, 0, { model->material->descriptorSet }, { });
					last.first = model;
				}

				auto transform = pair.second;
				if (transform != last.second)
				{
					auto matTransform = pair.second->getGlobalMatrix();	// M
					data[0].modelMatrix = matTransform;
					XrMatrix4x4f_Multiply(&(data[0].projectionView), &matProjectionView, (XrMatrix4x4f*)&matTransform);	// PVM
					commandBuffers[view].pushConstants<PushConstantData>(pipelineLayouts.worldPipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, data);
				}

				model->draw(commandBuffers[view]);
			}
		}
		// End Draw.
	}

	commandBuffers[view].endRenderPass();		// <========= Render Pass End.

#ifdef MIRROR_WINDOW
	if (view == mirrorView && !iconified)
	{
		vk::ImageMemoryBarrier barrier(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eTransferDstOptimal, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, mirrorSwapchain->images[mirrorImageIndex], { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
		barrier.image = mirrorSwapchain->images[mirrorImageIndex];
		barrier.oldLayout = vk::ImageLayout::eUndefined;
		barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
		commandBuffers[view].pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, { }, { }, { }, { barrier });
		
		barrier.image = swapChains[view]->images[imageIndex];
		barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		commandBuffers[view].pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, { }, { }, { }, { barrier });
		
		vk::ImageBlit blit(
			{ vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
			std::array<vk::Offset3D, 2>({ { 0, 0, 0 }, { static_cast<int32_t>(swapChains[view]->rect.extent.width) , static_cast<int32_t>(swapChains[view]->rect.extent.height), 1 } }),
			{ vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
			std::array<vk::Offset3D, 2>({ { 0, 0, 0 }, { static_cast<int32_t>(mirrorSwapchain->rect.extent.width) , static_cast<int32_t>(mirrorSwapchain->rect.extent.height), 1 } })
		);
		commandBuffers[view].blitImage(swapChains[view]->images[imageIndex], vk::ImageLayout::eTransferSrcOptimal, mirrorSwapchain->images[mirrorImageIndex], vk::ImageLayout::eTransferDstOptimal, { blit }, vk::Filter::eLinear);
		
		barrier.image = mirrorSwapchain->images[mirrorImageIndex];
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
		commandBuffers[view].pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, { }, { }, { }, { barrier });

		barrier.image = swapChains[view]->images[imageIndex];
		barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		commandBuffers[view].pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, { }, { }, { }, { barrier });
	}
#endif // MIRROR_WINDOW

	commandBuffers[view].end();		// <========= Command Buffer End.

	vk::SubmitInfo submitInfo{ }; submitInfo.commandBufferCount = 1; submitInfo.pCommandBuffers = &commandBuffers[view];
#ifdef MIRROR_WINDOW
	if (view == mirrorView && !iconified)
	{
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &mirrorImageAvailableSemaphore;
	}
#endif // MIRROR_WINDOW

	queue.submit(submitInfo, inFlights[view]);

#ifdef MIRROR_WINDOW
	if (view == mirrorView && !iconified)
	{
		vk::PresentInfoKHR presentInfo(0, nullptr, 1, &mirrorVkSwapchain, &mirrorImageIndex, nullptr);
		if (queue.presentKHR(presentInfo) != vk::Result::eSuccess)
		{
			throw std::runtime_error("Failed to present to mirror window.");
		}
	}
#endif
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
	if (!tinyobj::loadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str(), modelDirectory.c_str()))
	{
		throw std::runtime_error(warn + err);
	}
	LOG_SUCCESS();

	std::vector<hd::Material> mates;
	std::unordered_set<hd::MeshModel> modls;

	LOG_STEP("Vulkan", "Creating Textures");
	for (auto& material : materials)
	{
		auto diffuseTex = material.diffuse_texname == "" ? nullptr : std::make_shared<Texture>(textureDirectory + '/' + material.diffuse_texname);
		auto normalTex = material.bump_texname == "" ? nullptr : std::make_shared<Texture>(textureDirectory + '/' + material.bump_texname);
		mates.push_back(std::make_shared<Material>(diffuseTex, normalTex));
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
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2],
			};

			vertex.uv = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.f - attrib.texcoords[2 * index.texcoord_index + 1],
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
			modls.insert(std::make_shared<MeshModel>(vertices, indices, mates[materialId]));	// make current model.
			// no need to manually clear the containers.
		}
		else if (materialId != shapes[shapeCount].mesh.material_ids[0])	// the next shape has different texture.
		{
			modls.insert(std::make_shared<MeshModel>(vertices, indices, mates[materialId]));	// make current model.
			vertices.clear();	// clear.
			indices.clear();
			uniqueVertices.clear();
			materialId = shapes[shapeCount].mesh.material_ids[0];	// set material for the next shape.
		}
	}
	LOG_SUCCESS();

	LOG_INFO("Vulkan", std::format("Loaded {} textures and {} models", mates.size(), modls.size()), 0);

	hd::GameObject obj = std::make_shared<GameObject>();
	obj->models = std::move(modls);
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