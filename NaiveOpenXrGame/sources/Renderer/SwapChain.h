#pragma once

#include "mainCommon.h"

namespace Noxg
{
	MAKE_HANDLE(SwapChain);

	class SwapChain
	{
	public:
		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;
		SwapChain(SwapChain&&) = delete;
		SwapChain& operator=(SwapChain&&) = delete;

		SwapChain(vk::Device device, vk::RenderPass renderPass, std::vector<vk::Image>& images, vk::Format format, vk::Rect2D rect);
		~SwapChain();

		vk::RenderPassBeginInfo getRenderPassBeginInfo(uint32_t imageIndex);

		uint32_t length;

		vk::Viewport* getViewport();
		vk::Rect2D* getScissor();

	public:
		std::vector<vk::Image> images;
		std::vector<vk::ImageView> imageViews;
		std::vector<vk::Framebuffer> framebuffers;
		vk::Image depthImage;
		vk::DeviceMemory depthImageMemory;
		vk::ImageView depthImageView;
		vk::CommandBuffer commandBuffer;
		vk::Format format = vk::Format::eUndefined;
		vk::Rect2D rect = { };
		vk::Viewport viewport = { };

		vk::Device device;
		vk::RenderPass renderPass;
	};
}

