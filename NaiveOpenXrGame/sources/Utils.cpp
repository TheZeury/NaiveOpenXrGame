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

void Noxg::Utils::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels)
{
	auto commandBuffer = beginSingleTimeCommandBuffer();

	vk::ImageMemoryBarrier barrier({ }, { }, oldLayout, newLayout, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image, { vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1 });
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

void Noxg::Utils::generateMipmaps(vk::Image image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
	auto commandBuffer = beginSingleTimeCommandBuffer();

	vk::ImageMemoryBarrier barrier({ }, { }, { }, { }, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; ++i)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, { }, { }, { }, { barrier });

		vk::ImageBlit blit(
			{ vk::ImageAspectFlagBits::eColor, i - 1, 0, 1 }, 
			std::array<vk::Offset3D, 2>({ { 0, 0, 0 }, { mipWidth, mipHeight, 1 } }), 
			{ vk::ImageAspectFlagBits::eColor, i, 0, 1 }, 
			std::array<vk::Offset3D, 2>({ { 0, 0, 0 }, { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 } })
		);

		commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, { blit }, vk::Filter::eLinear);

		barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, { }, { }, { }, { barrier });

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, { }, { }, { }, { barrier });

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

vk::ImageView Noxg::Utils::createImageView(vk::Image& image, vk::Format format, uint32_t mipLevels, vk::ImageAspectFlags aspectFlags)
{
	vk::ImageViewCreateInfo viewInfo({}, image, vk::ImageViewType::e2D, format, { }, { aspectFlags, 0, mipLevels, 0, 1 });
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

vk::Format Noxg::Utils::findSupportedFormat(const std::vector<vk::Format> candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
	for(auto format : candidates)
	{
		auto properties = vkPhysicalDevice.getFormatProperties(format);
		if (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features)
		{
			return format;
		}
		if (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}
	throw std::runtime_error("Failed to find supported format.");
}

vk::Format Noxg::Utils::findDepthFormat()
{
	return findSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint }, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

bool Noxg::Utils::hasStencilComponent(vk::Format format)
{
	return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
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
