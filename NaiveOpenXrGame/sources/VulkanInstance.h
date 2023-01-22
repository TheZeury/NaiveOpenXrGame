#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace Noxg
{
	class VulkanInstance
	{
	public: // public functions.
		const int MAX_FRAMES_IN_FLIGHT = 2;

		VulkanInstance();
		~VulkanInstance();

		void Initialize(const xr::Instance& xrInstance, const xr::SystemId& xrSystemId);
		void CreateWindow();
		void CreateInstance();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapChainImageViews(std::vector<std::vector<xr::SwapchainImageVulkanKHR>>& swapChainImages, vk::Format format, std::vector<xr::Rect2Di> rects);
		void InitializeSession();
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		void CreateFrameBuffers();
		void CreateCommandPool();
		void AllocateCommandBuffers();
		void RenderView(xr::CompositionLayerProjectionView projectionView, uint32_t view, uint32_t imageIndex, vk::Format format);

		xr::GraphicsBindingVulkanKHR getGraphicsBinding();
	private: // help functions.
		std::vector<uint32_t> readFile(const std::string& filepath);

	private: // Owning. Responsible to destroy them. Or value types.
		GLFWwindow* window;
		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		uint32_t queueFamilyIndex;
		vk::Queue queue;
		vk::RenderPass renderPass;
		vk::PipelineLayout pipelineLayout;
		vk::Pipeline pipeline;
		std::vector<std::vector<vk::Framebuffer>> frameBuffers;
		vk::CommandPool commandPool;
		std::vector<vk::CommandBuffer> commandBuffers;
		vk::Semaphore drawDone;
		std::vector<std::vector<vk::Fence>> inFlights;
		std::vector<std::vector<vk::ImageView>> swapChainImageViews;
		vk::Format swapChainFormat;
		std::vector<xr::Rect2Di> swapChainRects;
		xr::DispatchLoaderDynamic dispather;
	
	private: // Not Owning. Don't try to destroy them. (But it's ok to destruct since they are handle classes instead of pointers.)
		xr::Instance xrInstance;
		xr::SystemId xrSystemId;
	};
}

