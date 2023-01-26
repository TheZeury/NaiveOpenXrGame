#pragma once

#include "mainCommon.h"

namespace Noxg
{
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
		vk::ImageView textureImageView;
		vk::Sampler textureSampler;
		vk::Image textureImage;
		vk::DeviceMemory textureImageMemory;
	};
}

