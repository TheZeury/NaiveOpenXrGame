#pragma once

#include "mainCommon.h"

namespace Noxg
{
	MAKE_HANDLE(Texture);

	class Texture
	{
	public:
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		Texture(std::string path);
		Texture(stb::stbi_uc* pixels, int width, int height, int channels);
		~Texture();

		void createTexture(stb::stbi_uc* pixels, int width, int height, int channels);

	public:
		std::vector<vk::DescriptorSet> descriptorSet;

	public:
		vk::ImageView textureImageView;
		vk::Sampler textureSampler;
		vk::Image textureImage;
		vk::DeviceMemory textureImageMemory;
	};
}

