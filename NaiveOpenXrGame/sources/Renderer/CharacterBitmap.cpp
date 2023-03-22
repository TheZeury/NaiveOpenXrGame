#include "CharacterBitmap.h"
#include "VulkanInstance.h"

Noxg::CharacterBitmap::CharacterBitmap(const std::string& fontPath, uint32_t width, uint32_t height, float pixelHeight)
{
	auto fontBuffer = VulkanInstance::readFile(fontPath);
	std::vector<stb::stbi_uc> singleChannelBitmapData(static_cast<size_t>(width * height));
	std::vector<stb::stbi_uc> rgbaBitmapData(static_cast<size_t>(width * height) * 4);
	stb::stbtt_BakeFontBitmap((unsigned char*)fontBuffer.data(), 0, pixelHeight, singleChannelBitmapData.data(), width, height, 0, 128, charInfos);
	for (size_t i = 0; i < singleChannelBitmapData.size(); ++i)
	{
		for(size_t j = 0; j < 4; ++j)
		{
			rgbaBitmapData[4 * i + j] = singleChannelBitmapData[i];
		}
	}
	bitmap = std::make_shared<Texture>(rgbaBitmapData.data(), static_cast<int32_t>(width), static_cast<int32_t>(height), 4);

	createDescriptorSet();
}

Noxg::CharacterBitmap::CharacterBitmap(hd::Texture texture) : bitmap{ texture }
{
	createDescriptorSet();
}

Noxg::CharacterBitmap::~CharacterBitmap()
{
	Utils::vkDevice.freeDescriptorSets(descriptorPool, 1, &descriptorSet);
}

vk::DescriptorSetLayout Noxg::CharacterBitmap::getDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding bitmapLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, { });
	std::array<vk::DescriptorSetLayoutBinding, 1> bindings = { bitmapLayoutBinding };
	vk::DescriptorSetLayoutCreateInfo layoutInfo({ }, bindings);

	return Utils::vkDevice.createDescriptorSetLayout(layoutInfo);
}

void Noxg::CharacterBitmap::createDescriptorSet()
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
