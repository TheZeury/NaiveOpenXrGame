#include "Material.h"

vk::DescriptorSetLayout Noxg::Material::getDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding propertyLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment, { });
	vk::DescriptorSetLayoutBinding diffuseMapLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, { });
	vk::DescriptorSetLayoutBinding normalMapLayoutBinding(2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, { });

	std::array<vk::DescriptorSetLayoutBinding, 3> bindings = { propertyLayoutBinding, diffuseMapLayoutBinding, normalMapLayoutBinding };
	vk::DescriptorSetLayoutCreateInfo layoutInfo({ }, bindings);

	return Utils::vkDevice.createDescriptorSetLayout(layoutInfo);
}

void Noxg::Material::createDescriptorSet()
{
	auto [stagingBuffer, stagingBufferMemory] = 
		Utils::CreateBuffer(sizeof(propertyBufferObject), 1, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	
	void* mappingMemory = Utils::mapMemory(stagingBufferMemory);
	std::memcpy(mappingMemory, &propertyBufferObject, sizeof(MaterialPropertyBufferObject));
	Utils::unmapMemory(stagingBufferMemory);

	std::tie(propertyBuffer, propertyBufferMemory) = 
		Utils::CreateBuffer(sizeof(propertyBufferObject), 1, vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);

	Utils::copyBuffer(propertyBuffer, stagingBuffer, sizeof(MaterialPropertyBufferObject));
	Utils::destroyBuffer(stagingBuffer, stagingBufferMemory);

	std::array<vk::DescriptorSetLayout, 1> layouts{ materialSetLayout };
	vk::DescriptorSetAllocateInfo allocateInfo(descriptorPool, layouts);
	descriptorSet = Utils::vkDevice.allocateDescriptorSets(allocateInfo)[0];

	vk::DescriptorBufferInfo propertyBufferInfo(propertyBuffer, { }, sizeof(MaterialPropertyBufferObject));
	vk::DescriptorImageInfo diffuseMapInfo;
	vk::DescriptorImageInfo normalMapInfo;

	std::vector<vk::WriteDescriptorSet> descriptorWrites;

	{
		descriptorWrites.push_back({ descriptorSet, 0, 0, 1, vk::DescriptorType::eUniformBuffer, { }, &propertyBufferInfo });
	}

	if (propertyBufferObject.enableDiffuseMap)
	{
		diffuseMapInfo = { diffuseMap->textureSampler, diffuseMap->textureImageView, vk::ImageLayout::eShaderReadOnlyOptimal };
		descriptorWrites.push_back({ descriptorSet, 1, 0, 1, vk::DescriptorType::eCombinedImageSampler, &diffuseMapInfo });
	}
	else
	{
		diffuseMapInfo = { Texture::empty->textureSampler, Texture::empty->textureImageView, vk::ImageLayout::eShaderReadOnlyOptimal };
		descriptorWrites.push_back({ descriptorSet, 1, 0, 1, vk::DescriptorType::eCombinedImageSampler, &diffuseMapInfo });
	}

	if (propertyBufferObject.enableNormalMap)
	{
		normalMapInfo = { normalMap->textureSampler, normalMap->textureImageView, vk::ImageLayout::eShaderReadOnlyOptimal };
		descriptorWrites.push_back({ descriptorSet, 2, 0, 1, vk::DescriptorType::eCombinedImageSampler, &normalMapInfo });
	}
	else
	{
		normalMapInfo = { Texture::empty->textureSampler, Texture::empty->textureImageView, vk::ImageLayout::eShaderReadOnlyOptimal };
		descriptorWrites.push_back({ descriptorSet, 2, 0, 1, vk::DescriptorType::eCombinedImageSampler, &normalMapInfo });
	}

	Utils::vkDevice.updateDescriptorSets(descriptorWrites, { });
}
