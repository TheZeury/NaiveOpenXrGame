#pragma once

#include "mainCommon.h"
#include "Material.h"
#include "CharacterBitmap.h"
#include "IModel.h"

namespace Noxg
{
	MAKE_HANDLE(TextModel);

	class TextModel : public IModel
	{
	public:
		struct TextVertex
		{
			glm::vec3 position;
			glm::vec2 textureUv;
			glm::vec2 bitmapUv;
			glm::vec3 normal;
			glm::vec3 tangent;
			glm::vec3 bitangent;

			static std::array<vk::VertexInputBindingDescription, 1> getBindingDescriptions()
			{
				return {
					vk::VertexInputBindingDescription{ 0, sizeof(TextVertex), vk::VertexInputRate::eVertex },
				};
			}

			static std::array<vk::VertexInputAttributeDescription, 6> getAttributeDescriptions()
			{
				return {
					vk::VertexInputAttributeDescription{ 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(TextVertex, position) },
					vk::VertexInputAttributeDescription{ 1, 0, vk::Format::eR32G32Sfloat, offsetof(TextVertex, textureUv) },
					vk::VertexInputAttributeDescription{ 2, 0, vk::Format::eR32G32Sfloat, offsetof(TextVertex, bitmapUv) },
					vk::VertexInputAttributeDescription{ 3, 0, vk::Format::eR32G32B32Sfloat, offsetof(TextVertex, normal) },
					vk::VertexInputAttributeDescription{ 4, 0, vk::Format::eR32G32B32Sfloat, offsetof(TextVertex, tangent) },
					vk::VertexInputAttributeDescription{ 5, 0, vk::Format::eR32G32B32Sfloat, offsetof(TextVertex, bitangent) },
				};
			}

			bool operator==(const TextVertex& other) const
			{
				return position == other.position && textureUv == other.textureUv && bitmapUv == other.bitmapUv && normal == other.normal;
			}
		};

		TextModel(const std::string& text, hd::Material material, hd::CharacterBitmap bitmap, float height = 0.1f);

		void createTextModel(const std::string& text, float height);

		void bind(vk::CommandBuffer& commandBuffer);
		void draw(vk::CommandBuffer& commandBuffer);

	public:
		hd::Material material;
		hd::CharacterBitmap bitmap;

	private:
		uint32_t vertexCount;
		uint32_t indexCount;
		vk::Buffer vertexBuffer;
		vk::DeviceMemory vertexBufferMemory;
		vk::Buffer indexBuffer;
		vk::DeviceMemory indexBufferMemory;
	};
}

namespace std
{
	template<>
	struct hash<Noxg::TextModel::TextVertex>
	{
		size_t operator()(Noxg::TextModel::TextVertex const& vertex) const
		{
			return (((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.textureUv) << 1) >> 1) ^
				(hash<glm::vec2>()(vertex.bitmapUv) << 1);
		}
	};
}

