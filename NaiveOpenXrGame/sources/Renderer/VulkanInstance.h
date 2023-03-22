#pragma once

#include "mainCommon.h"
#include "GraphicsInstance.h"
#include "Bricks/GameObject.h"
#include "Bricks/Scene.h"
#include "SwapChain.h"
#include "UIElement.h"

namespace Noxg
{
	MAKE_HANDLE(VulkanInstance);

	class VulkanInstance : public GraphicsInstance
	{
	public:
		struct PushConstantData
		{
			XrMatrix4x4f projectionView;
			glm::mat4 modelMatrix;
		};

	public: // public functions.
		const int MAX_FRAMES_IN_FLIGHT = 2;

		VulkanInstance();
		~VulkanInstance();

		virtual void Initialize(rf::XrInstance xrInstance) override;
		virtual void CleanUpInstance() override;
		virtual void CleanUpSession() override;
#ifdef MIRROR_WINDOW
		void CreateWindow();
		GLFWwindow* getWindow() { return window; }
#endif
		void CreateInstance();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		virtual void CreateSwapChainImageViews(std::vector<std::vector<xr::SwapchainImageVulkanKHR>>& swapChainImages, vk::Format format, std::vector<xr::Rect2Di> rects) override;
		virtual void InitializeSession() override;
		void CreateRenderPass(vk::Format format);
		void CreateDescriptors();
		void CreateGraphicsPipelines();
		void CreateCommandPool();
		void AllocateCommandBuffers();
		virtual void RenderView(xr::CompositionLayerProjectionView projectionView, uint32_t view, uint32_t imageIndex, vk::Format format) override;

		virtual void addScene(rf::Scene scene) override;

		virtual hd::GameObject loadGameObjectFromFiles(std::string name) override;	// May creates multiple textures and models, but only a single gameObject.
		virtual xr::GraphicsBindingVulkanKHR getGraphicsBinding() override;
	public: // help functions.
		static std::vector<uint32_t> readFile(const std::string& filepath);

	private: // Owning. Responsible to destroy them. Or value types.
		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		uint32_t queueFamilyIndex;
		vk::Queue queue;
		vk::RenderPass renderPass;
		vk::DescriptorPool descriptorPool;
		struct {
			vk::PipelineLayout worldPipelineLayout;
		} pipelineLayouts;
		struct {
			vk::Pipeline meshPipeline;
			vk::Pipeline textPipeline;
			vk::Pipeline wireframePipeline;
			vk::Pipeline uiPipeline;
		} pipelines;
		vk::CommandPool commandPool;
		std::vector<vk::CommandBuffer> commandBuffers;
		// std::vector<std::vector<hd::MeshModel>> preservedModels;	// models are preserved by a commandBuffer when they are being drawn.
		vk::Semaphore drawDone;
		std::vector<vk::Fence> inFlights;
		std::vector<hd::SwapChain> swapChains;
		xr::DispatchLoaderDynamic dispatcher;

#ifdef MIRROR_WINDOW
		GLFWwindow* window = nullptr;
		vk::SurfaceKHR mirrorSurface;
		vk::SwapchainKHR mirrorVkSwapchain;
		hd::SwapChain mirrorSwapchain;
		vk::Semaphore mirrorImageAvailableSemaphore;
		uint32_t mirrorView = 0;
		uint32_t mirrorImageHeight = 1440ui32;
#endif
	
	private: // Not Owning. Don't try to destroy them. (But it's ok to destruct since they are handle classes instead of pointers.)
		xr::Instance openXrInstance;
		xr::SystemId openXrSystemId;
		std::list<rf::Scene> scenes;
	};
}

