#pragma once

#include "mainCommon.h"
#include "Texture.h"
#include "Utils.h"

namespace Noxg
{
	MAKE_HANDLE(Material);

	struct MaterialPropertyBufferObject
	{
		alignas(4) vk::Bool32 enableDiffuseMap;
		alignas(4) vk::Bool32 enableNormalMap;
		alignas(16) glm::vec4 defaultColor;
	};

	class Material
	{
	public:
		Material(const Material&) = delete;
		Material& operator=(const Material&) = delete;

		// Material with textures. some textures can be nullptr, but if all of them are nullptr, consider using Material(glm::vec4 color).
		Material(hd::Texture diffuse = nullptr, hd::Texture normal = nullptr, glm::vec4 defaultColor = { 1.f, 1.f, 1.f, 1.f }) 
			: diffuseMap{ diffuse }, normalMap{ normal }, propertyBufferObject{ static_cast<vk::Bool32>(diffuse != nullptr), static_cast<vk::Bool32>(normal != nullptr), defaultColor } 
		{
			createDescriptorSet();
		}
		// No texture single color material.
		Material(glm::vec4 defaultColor) : propertyBufferObject { VK_FALSE, VK_FALSE, defaultColor } 
		{
			createDescriptorSet();
		}
		~Material()
		{
			Utils::vkDevice.freeDescriptorSets(descriptorPool, 1, &descriptorSet);
			Utils::destroyBuffer(propertyBuffer, propertyBufferMemory);
		}

		static vk::DescriptorSetLayout getDescriptorSetLayout();

		static inline vk::DescriptorSetLayout materialSetLayout;

		void createDescriptorSet();

	public:
		inline static vk::DescriptorPool descriptorPool;

	public:
		vk::DescriptorSet descriptorSet;
		MaterialPropertyBufferObject propertyBufferObject;
		vk::Buffer propertyBuffer;
		vk::DeviceMemory propertyBufferMemory;
		hd::Texture diffuseMap = nullptr;
		hd::Texture normalMap = nullptr;
	};
}

