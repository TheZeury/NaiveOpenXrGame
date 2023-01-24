#pragma once

#include "mainCommon.h"

namespace Noxg
{
	static class Utils
	{
	public:
		static void passInGraphicsInformation(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device, vk::CommandPool commandPool, vk::Queue queue);
		static void passInXrInformation();
		static std::tuple<vk::Buffer, vk::DeviceMemory> AllocateBuffer
			(vk::DeviceSize instanceSize, uint32_t instanceCount, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::DeviceSize minOffsetAlignment = 1);
		static void* mapMemory(vk::DeviceMemory memory, vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
		static void unmapMemory(vk::DeviceMemory memory);
		static void copyBuffer(vk::Buffer& dstBuffer, vk::Buffer& srcBuffer, vk::DeviceSize size);
		static void destroyBuffer(vk::Buffer& buffer, vk::DeviceMemory& memory);
		static vk::CommandBuffer beginSingleTimeCommandBuffer();
		static void endSingleTimeCommandBuffer(vk::CommandBuffer& commandBuffer);
	private:
		static vk::DeviceSize getAlignment(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment);
	private:
		inline static vk::Instance vkInstance;
		inline static vk::PhysicalDevice vkPhysicalDevice;
		inline static vk::Device vkDevice;
		inline static vk::CommandPool vkCommandPool;
		inline static vk::Queue vkQueue;
	};
}
