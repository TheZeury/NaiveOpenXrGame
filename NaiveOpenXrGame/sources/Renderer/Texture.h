#pragma once

#include "mainCommon.h"

namespace Noxg
{
	class Texture_T
	{
	public:
		Texture_T(const Texture_T&) = delete;
		Texture_T& operator=(const Texture_T&) = delete;

		Texture_T(std::string path);
		Texture_T(stb::stbi_uc* pixels, int width, int height, int channels);
		~Texture_T();

		void createTexture(stb::stbi_uc* pixels, int width, int height, int channels);

	public:
		std::vector<vk::DescriptorSet> descriptorSet;

	public:
		vk::ImageView textureImageView;
		vk::Sampler textureSampler;
		vk::Image textureImage;
		vk::DeviceMemory textureImageMemory;
	};

	using Texture = std::shared_ptr<Texture_T>;
}

