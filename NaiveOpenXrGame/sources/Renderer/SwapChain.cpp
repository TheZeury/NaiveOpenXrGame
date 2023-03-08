#include "SwapChain.h"
#include "Utils.h"

Noxg::SwapChain::SwapChain(vk::Device device, vk::RenderPass renderPass, std::vector<vk::Image>& images, vk::Format format, vk::Rect2D rect)
{
	this->length = static_cast<uint32_t>(images.size());
	this->format = format;
	this->rect = rect;
	this->device = device;
	this->renderPass = renderPass;

	this->images = images;

	// ImageViews.
	for (auto& image : images)
	{
		vk::ImageViewCreateInfo createInfo{ {}, image, vk::ImageViewType::e2D, format, { }, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };
		imageViews.push_back(device.createImageView(createInfo));
	}

	// Depth.
	auto depthFormat = Utils::findDepthFormat();
	std::array<uint32_t, 1> queueFamilyIndices = { 0 };
	vk::ImageCreateInfo imageInfo({ }, vk::ImageType::e2D, depthFormat, { static_cast<uint32_t>(rect.extent.width), static_cast<uint32_t>(rect.extent.height), 1 }, 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, queueFamilyIndices, vk::ImageLayout::eUndefined);
	std::tie(depthImage,depthImageMemory) = Utils::CreateImage(imageInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);
	depthImageView = Utils::createImageView(depthImage, depthFormat, 1, vk::ImageAspectFlagBits::eDepth);

	// Framebuffer.
	for (auto& view : imageViews)
	{
		std::array<vk::ImageView, 2> attachments = { view, depthImageView };
		vk::FramebufferCreateInfo createInfo({ }, renderPass, attachments, rect.extent.width, rect.extent.height, 1);
		framebuffers.push_back(device.createFramebuffer(createInfo));
	}
}

Noxg::SwapChain::~SwapChain()
{
	// Framebuffers.
	for (auto& framebuffer : framebuffers)
	{
		device.destroyFramebuffer(framebuffer);
	}

	// Depth.
	Utils::destroyImage(depthImage, depthImageMemory);
	device.destroyImageView(depthImageView);

	// ImageViews.
	for (auto& view : imageViews)
	{
		device.destroyImageView(view);
	}
}

vk::RenderPassBeginInfo Noxg::SwapChain::getRenderPassBeginInfo(uint32_t imageIndex)
{
	std::array<vk::ClearValue, 2> clearValues = {
		vk::ClearValue{ vk::ClearColorValue{ std::array<float, 4>{ 0.7f, 0.8f, 0.5f, 1.f } } },
		vk::ClearValue{ vk::ClearDepthStencilValue{ 1.f, 0 } },
	};
	vk::RenderPassBeginInfo renderPassInfo(renderPass, framebuffers[imageIndex], { { }, { rect.extent.width, rect.extent.height } }, clearValues);
	return renderPassInfo;
}

vk::Viewport* Noxg::SwapChain::getViewport()
{
	viewport = vk::Viewport{ 0.f, 0.f, static_cast<float>(rect.extent.width), static_cast<float>(rect.extent.height), 0.f, 1.f };
	return &viewport;
}

vk::Rect2D* Noxg::SwapChain::getScissor()
{
	return &rect;
}
