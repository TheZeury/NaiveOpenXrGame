#include "Utils.h"

void Noxg::Utils::passInGraphicsInformation(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device, vk::CommandPool commandPool, vk::Queue queue)
{
	vkInstance = instance;
	vkPhysicalDevice = physicalDevice;
	vkDevice = device;
	vkCommandPool = commandPool;
	vkQueue = queue;
}

std::tuple<vk::Buffer, vk::DeviceMemory> Noxg::Utils::CreateBuffer(vk::DeviceSize instanceSize, uint32_t instanceCount, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::DeviceSize minOffsetAlignment)
{
	vk::DeviceSize bufferSize = getAlignment(instanceSize, minOffsetAlignment) * instanceCount;

	// Buffer
	vk::BufferCreateInfo bufferInfo({ }, bufferSize, usageFlags, vk::SharingMode::eExclusive);
	vk::Buffer buffer = vkDevice.createBuffer(bufferInfo);

	// Memory
	vk::MemoryRequirements memoryRequirements = vkDevice.getBufferMemoryRequirements(buffer);
	vk::MemoryAllocateInfo allInfo(memoryRequirements.size, findMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlags));
	vk::DeviceMemory memory = vkDevice.allocateMemory(allInfo);

	// Bind
	vkDevice.bindBufferMemory(buffer, memory, 0);

	// Return
	return std::make_tuple(buffer, memory);
}

std::tuple<vk::Image, vk::DeviceMemory> Noxg::Utils::CreateImage(vk::ImageCreateInfo& imageInfo, vk::MemoryPropertyFlags memoryPropertyFlags)
{
	// Image
	vk::Image image = vkDevice.createImage(imageInfo);

	// Memory
	vk::MemoryRequirements memoryRequirement = vkDevice.getImageMemoryRequirements(image);
	vk::MemoryAllocateInfo allocateInfo(memoryRequirement.size, findMemoryType(memoryRequirement.memoryTypeBits, memoryPropertyFlags));
	vk::DeviceMemory memory = vkDevice.allocateMemory(allocateInfo);

	// Bind
	vkDevice.bindImageMemory(image, memory, 0);

	// Return
	return std::make_tuple(image, memory);
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

void Noxg::Utils::copyBufferToImage(vk::Image& dstImage, vk::Buffer& srcBuffer, uint32_t width, uint32_t height)
{
	auto commandBuffer = beginSingleTimeCommandBuffer();

	std::array<vk::BufferImageCopy, 1> imageCopy = {
		vk::BufferImageCopy{ 0, 0, 0, { vk::ImageAspectFlagBits::eColor, 0, 0, 1 }, { 0, 0, 0 }, { width, height, 1 } },
	};
	commandBuffer.copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, imageCopy);

	endSingleTimeCommandBuffer(commandBuffer);
}

void Noxg::Utils::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
	auto commandBuffer = beginSingleTimeCommandBuffer();

	vk::ImageMemoryBarrier barrier({ }, { }, oldLayout, newLayout, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
	vk::PipelineStageFlags sourceStage, destinationStage;
	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = { };
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else
	{
		throw std::invalid_argument("Unsupported layout transition.");
	}
	std::array<vk::ImageMemoryBarrier, 1> barriers = {
		barrier,
	};

	commandBuffer.pipelineBarrier(sourceStage, destinationStage, { }, { }, { }, barriers);

	endSingleTimeCommandBuffer(commandBuffer);
}

void Noxg::Utils::destroyBuffer(vk::Buffer& buffer, vk::DeviceMemory& memory)
{
	vkDevice.destroyBuffer(buffer);
	vkDevice.freeMemory(memory);
}

void Noxg::Utils::destroyImage(vk::Image& image, vk::DeviceMemory& memory)
{
	vkDevice.destroyImage(image);
	vkDevice.freeMemory(memory);
}

vk::ImageView Noxg::Utils::createImageView(vk::Image& image, vk::Format format)
{
	vk::ImageViewCreateInfo viewInfo({}, image, vk::ImageViewType::e2D, format, { }, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
	return vkDevice.createImageView(viewInfo);
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

uint32_t Noxg::Utils::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags memoryPropertyFlags)
{
	auto memoryProperties = vkPhysicalDevice.getMemoryProperties();
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((typeFilter & (1 << i)) && ((memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags))
		{
			return i;
		}
	}
	throw std::runtime_error("Failed to find memory type.");
	return 0;
}
