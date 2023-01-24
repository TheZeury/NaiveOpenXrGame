#include "Utils.h"

void Noxg::Utils::passInGraphicsInformation(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device, vk::CommandPool commandPool, vk::Queue queue)
{
	vkInstance = instance;
	vkPhysicalDevice = physicalDevice;
	vkDevice = device;
	vkCommandPool = commandPool;
	vkQueue = queue;
}

std::tuple<vk::Buffer, vk::DeviceMemory> Noxg::Utils::AllocateBuffer(vk::DeviceSize instanceSize, uint32_t instanceCount, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::DeviceSize minOffsetAlignment)
{
	vk::DeviceSize bufferSize = getAlignment(instanceSize, minOffsetAlignment) * instanceCount;
	auto memoryProperties = vkPhysicalDevice.getMemoryProperties();

	// Buffer
	vk::BufferCreateInfo bufferInfo({ }, bufferSize, usageFlags, vk::SharingMode::eExclusive);
	vk::Buffer buffer = vkDevice.createBuffer(bufferInfo);

	// Memory
	vk::MemoryRequirements memoryRequirements = vkDevice.getBufferMemoryRequirements(buffer);
	auto typeFilter = memoryRequirements.memoryTypeBits;
	uint32_t memoryTypeIndex = -1;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((typeFilter & (1 << i)) && ((memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags))
		{
			memoryTypeIndex = i;
			break;
		}
	}
	if (memoryTypeIndex < 0) throw std::runtime_error("Failed to find memory type.");
	vk::MemoryAllocateInfo allInfo(memoryRequirements.size, memoryTypeIndex);
	vk::DeviceMemory memory = vkDevice.allocateMemory(allInfo);

	// Bind
	vkDevice.bindBufferMemory(buffer, memory, 0);

	// Return
	return std::make_tuple(buffer, memory);
}

void* Noxg::Utils::mapMemory(vk::DeviceMemory memory, vk::DeviceSize size, vk::DeviceSize offset)
{
	return vkDevice.mapMemory(memory, offset, size);
}

void Noxg::Utils::unmapMemory(vk::DeviceMemory memory)
{
	vkDevice.unmapMemory(memory);
}

void Noxg::Utils::copyBuffer(vk::Buffer& dstBuffer, vk::Buffer& srcBuffer, vk::DeviceSize size)
{
	auto commandBuffer = beginSingleTimeCommandBuffer();

	std::array<vk::BufferCopy, 1> bufferCopies = { vk::BufferCopy{ 0, 0, size } };
	commandBuffer.copyBuffer(srcBuffer, dstBuffer, bufferCopies);

	endSingleTimeCommandBuffer(commandBuffer);
}

void Noxg::Utils::destroyBuffer(vk::Buffer& buffer, vk::DeviceMemory& memory)
{
	vkDevice.destroyBuffer(buffer);
	vkDevice.freeMemory(memory);
}

vk::CommandBuffer Noxg::Utils::beginSingleTimeCommandBuffer()
{
	vk::CommandBufferAllocateInfo allocateInfo(vkCommandPool, vk::CommandBufferLevel::ePrimary, 1);
	auto commandBuffer = vkDevice.allocateCommandBuffers(allocateInfo)[0];

	vk::CommandBufferBeginInfo beginInfo({ });
	commandBuffer.begin(beginInfo);
	
	return commandBuffer;
}

void Noxg::Utils::endSingleTimeCommandBuffer(vk::CommandBuffer& commandBuffer)
{
	commandBuffer.end();
	std::array<vk::CommandBuffer, 1> commandBuffers = { commandBuffer };

	vk::SubmitInfo submitInfo({ }, { }, commandBuffers);
	std::array<vk::SubmitInfo, 1> infos = { submitInfo };
	vkQueue.submit(infos);
	vkQueue.waitIdle();

	vkDevice.freeCommandBuffers(vkCommandPool, commandBuffers);
}

inline vk::DeviceSize Noxg::Utils::getAlignment(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment) {
	if (minOffsetAlignment > 0) {
		return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
	}
	return instanceSize;
}
