#pragma once

#include "mainCommon.h"
#include "GraphicsInstance.h"
#include "Bricks/GameObject.h"
#include "Bricks/Scene.h"

#include <list>

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
		void CreateWindow();
		void CreateInstance();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		virtual void CreateSwapChainImageViews(std::vector<std::vector<xr::SwapchainImageVulkanKHR>>& swapChainImages, vk::Format format, std::vector<xr::Rect2Di> rects) override;
		virtual void InitializeSession() override;
		void CreateRenderPass();
		void CreateDescriptors();
		void CreateGraphicsPipeline();
		void CreateFrameBuffers();
		void CreateCommandPool();
		void CreateDepthResources();
		void AllocateCommandBuffers();
		virtual void RenderView(xr::CompositionLayerProjectionView projectionView, uint32_t view, uint32_t imageIndex, vk::Format format) override;

		virtual void addTexture(hd::Texture texture) override;
		virtual void addModel(hd::MeshModel model) override;
		virtual void addScene(rf::Scene scene) override;
		virtual hd::GameObject loadGameObjectFromFiles(std::string name) override;	// May creates multiple textures and models, but only a single gameObject.
		virtual xr::GraphicsBindingVulkanKHR getGraphicsBinding() override;
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
		vk::DescriptorSetLayout textureSetLayout;
		vk::DescriptorPool descriptorPool;
		std::vector<vk::Image> depthImages;
		std::vector<vk::DeviceMemory> depthImageMemories;
		std::vector<vk::ImageView> depthImageViews;
		vk::PipelineLayout pipelineLayout;
		vk::Pipeline pipeline;
		std::vector<std::vector<vk::Framebuffer>> frameBuffers;
		vk::CommandPool commandPool;
		std::vector<vk::CommandBuffer> commandBuffers;
		vk::Semaphore drawDone;
		std::vector<vk::Fence> inFlights;
		std::vector<std::vector<vk::ImageView>> swapChainImageViews;
		vk::Format swapChainFormat;
		std::vector<xr::Rect2Di> swapChainRects;
		xr::DispatchLoaderDynamic dispather;

		std::vector<hd::MeshModel> models;
		std::vector<hd::Texture> textures;

#ifdef MIRROR_WINDOW
		vk::SurfaceKHR mirrorSurface;
#endif
	
	private: // Not Owning. Don't try to destroy them. (But it's ok to destruct since they are handle classes instead of pointers.)
		xr::Instance openXrInstance;
		xr::SystemId openXrSystemId;
		std::list<rf::Scene> scenes;
	};
}

