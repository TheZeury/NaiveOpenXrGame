#include "CharactorBitmap.h"
#include "VulkanInstance.h"

Noxg::CharactorBitmap::CharactorBitmap(const std::string& fontPath, uint32_t width, uint32_t height, float pixelHeight)
{
	auto fontBuffer = VulkanInstance::readFile(fontPath);
	std::vector<stb::stbi_uc> bitmapData(static_cast<size_t>(width * height));
	stb::stbtt_BakeFontBitmap((unsigned char*)fontBuffer.data(), 0, pixelHeight, bitmapData.data(), width, height, 0, 128, charInfos);
	bitmap = std::make_shared<Texture>(bitmapData.data(), static_cast<int32_t>(width), static_cast<int32_t>(height), 1);

	createDescriptorSet();
}

Noxg::CharactorBitmap::~CharactorBitmap()
{
	Utils::vkDevice.freeDescriptorSets(descriptorPool, 1, &descriptorSet);
}

vk::DescriptorSetLayout Noxg::CharactorBitmap::getDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding bitmapLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, { });
	std::array<vk::DescriptorSetLayoutBinding, 1> bindings = { bitmapLayoutBinding };
	vk::DescriptorSetLayoutCreateInfo layoutInfo({ }, bindings);

	return Utils::vkDevice.createDescriptorSetLayout(layoutInfo);
}

void Noxg::CharactorBitmap::createDescriptorSet()
{
	std::array<vk::DescriptorSetLayout, 1> layouts{ bitmapSetLayout };
	vk::DescriptorSetAllocateInfo allocateInfo(descriptorPool, layouts);
	descriptorSet = Utils::vkDevice.allocateDescriptorSets(allocateInfo)[0];

	vk::DescriptorImageInfo bitmapInfo{ bitmap->textureSampler, bitmap->textureImageView, vk::ImageLayout::eShaderReadOnlyOptimal };
	std::vector<vk::WriteDescriptorSet> descriptorWrites = {
		{ descriptorSet, 0, 0, 1, vk::DescriptorType::eCombinedImageSampler, &bitmapInfo },
	};

	Utils::vkDevice.updateDescriptorSets(descriptorWrites, { });
}
