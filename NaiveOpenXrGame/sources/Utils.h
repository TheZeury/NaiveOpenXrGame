#pragma once

#include "mainCommon.h"

namespace Noxg
{
	class Utils
	{
	public:
		static void passInGraphicsInformation(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device, vk::CommandPool commandPool, vk::Queue queue);
		static void passInXrInformation();
		static std::tuple<vk::Buffer, vk::DeviceMemory> CreateBuffer
			(vk::DeviceSize instanceSize, uint32_t instanceCount, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::DeviceSize minOffsetAlignment = 1);
		static std::tuple<vk::Image, vk::DeviceMemory> CreateImage(vk::ImageCreateInfo& imageInfo, vk::MemoryPropertyFlags memoryPropertyFlags);
		static void* mapMemory(vk::DeviceMemory memory, vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
		static void unmapMemory(vk::DeviceMemory memory);
		static void copyBuffer(vk::Buffer& dstBuffer, vk::Buffer& srcBuffer, vk::DeviceSize size);
		static void copyBufferToImage(vk::Image& dstImage, vk::Buffer& srcBuffer, uint32_t width, uint32_t height);
		static void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels = 1);
		static void generateMipmaps(vk::Image image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
		static void destroyBuffer(vk::Buffer& buffer, vk::DeviceMemory& memory);
		static void destroyImage(vk::Image& image, vk::DeviceMemory& memory);
		static vk::ImageView createImageView(vk::Image& image, vk::Format format, uint32_t mipLevels = 1, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor);
		static vk::CommandBuffer beginSingleTimeCommandBuffer();
		static void endSingleTimeCommandBuffer(vk::CommandBuffer& commandBuffer);
		static vk::Format findSupportedFormat(const std::vector<vk::Format> candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
		static vk::Format findDepthFormat();
		static bool hasStencilComponent(vk::Format format);
	private:
		static vk::DeviceSize getAlignment(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment);
		static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags memoryPropertyFlags);
	public:
		inline static vk::Instance vkInstance;
		inline static vk::PhysicalDevice vkPhysicalDevice;
		inline static vk::Device vkDevice;
		inline static vk::CommandPool vkCommandPool;
		inline static vk::Queue vkQueue;

		inline static std::array<xr::SpaceLocation, 2> handLocations;
		inline static std::array<xr::ActionStateFloat, 2> triggerStates;
		inline static std::array<bool, 2> released = { true, true };

		inline static float maxAnisotrophy;
	};
}
