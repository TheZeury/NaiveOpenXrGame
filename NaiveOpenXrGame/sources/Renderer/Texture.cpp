#include "Texture.h"
#include "Utils.h"

Noxg::Texture::Texture(std::string path)
{
	int width, height, channels;
	stb::stbi_uc* pixels = stb::stbi_load(path.c_str(), &width, &height, &channels, stb::STBI_rgb_alpha);
	if (!pixels) throw std::runtime_error("Failed to load texture image from file \"" + path + "\".");
	createTexture(pixels, width, height, 4);
	stb::stbi_image_free(pixels);
}

Noxg::Texture::Texture(stb::stbi_uc* pixels, int width, int height, int channels)
{
	createTexture(pixels, width, height, channels);
}

Noxg::Texture::~Texture()
{
	Utils::vkDevice.destroySampler(textureSampler);
	Utils::vkDevice.destroyImageView(textureImageView);
	Utils::destroyImage(textureImage, textureImageMemory);
}

void Noxg::Texture::createTexture(stb::stbi_uc* pixels, int width, int height, int channels)
{
	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
	vk::DeviceSize imageSize = sizeof(stb::stbi_uc) * width * height * channels;
	auto [stagingBuffer, stagingBufferMemory] = 
		Utils::CreateBuffer(sizeof(stb::stbi_uc), width * height * channels, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	auto mappingMemory = Utils::mapMemory(stagingBufferMemory);
	std::memcpy(mappingMemory, pixels, static_cast<size_t>(imageSize));
	Utils::unmapMemory(stagingBufferMemory);
	
	vk::Format imageFormat;
	if (channels == 1)
	{
		imageFormat = vk::Format::eR8Unorm;
	}
	else if (channels == 4)
	{
		imageFormat = vk::Format::eR8G8B8A8Srgb;
	}
	else
	{
		throw std::runtime_error(std::format("Unsupported image channels({}) for a texture. Must be 1 for grayscale or 4 for rgba.", channels));
	}

	std::array<uint32_t, 1> queueFamilyIndices = { 0 };
	vk::ImageCreateInfo imageInfo({ }, vk::ImageType::e2D, imageFormat, { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 }, mipLevels, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, queueFamilyIndices, vk::ImageLayout::eUndefined);

	std::tie(textureImage, textureImageMemory) = Utils::CreateImage(imageInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);

	Utils::transitionImageLayout(textureImage, imageFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
	Utils::copyBufferToImage(textureImage, stagingBuffer, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	Utils::destroyBuffer(stagingBuffer, stagingBufferMemory);
	//Utils::transitionImageLayout(textureImage, imageFormat, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	Utils::generateMipmaps(textureImage, width, height, mipLevels);

	textureImageView = Utils::createImageView(textureImage, imageFormat, mipLevels);

	vk::SamplerCreateInfo samplerInfo({ }, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, 0.f, VK_TRUE, Utils::maxAnisotrophy, VK_FALSE, vk::CompareOp::eAlways, 0.f, static_cast<float>(mipLevels), vk::BorderColor::eIntOpaqueBlack, VK_FALSE);
	textureSampler = Utils::vkDevice.createSampler(samplerInfo);
}
