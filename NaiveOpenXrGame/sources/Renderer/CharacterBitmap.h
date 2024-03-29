#pragma once

#include "mainCommon.h"
#include "Texture.h"

namespace Noxg
{
	using CharInfo = stb::stbtt_bakedchar;

	MAKE_HANDLE(CharacterBitmap);

	class CharacterBitmap
	{
	public:
		CharacterBitmap(const std::string& fontPath, uint32_t width = 1024, uint32_t height = 1024, float pixelHeight = 150.f);
		CharacterBitmap(hd::Texture texture);
		~CharacterBitmap();

		hd::Texture bitmap;
		CharInfo charInfos[128];

		static vk::DescriptorSetLayout getDescriptorSetLayout();

		static inline vk::DescriptorSetLayout bitmapSetLayout;

		void createDescriptorSet();

	public:
		inline static vk::DescriptorPool descriptorPool;
		vk::DescriptorSet descriptorSet;
	};
}

