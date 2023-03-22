#pragma once

#include "mainCommon.h"
#include "Material.h"
#include "CharacterBitmap.h"

namespace Noxg
{
	MAKE_HANDLE(UIElement);

	class UIElement
	{
	public:
		struct UIVertex
		{
			glm::vec2 position;
			glm::vec2 uv;
			glm::vec4 color;

			static auto getBindingDescriptions() -> std::array<vk::VertexInputBindingDescription, 1>
			{
				return {
					vk::VertexInputBindingDescription{ 0, sizeof(UIVertex), vk::VertexInputRate::eVertex },
				};
			}

			static auto getAttributeDescriptions() -> std::array<vk::VertexInputAttributeDescription, 3>
			{
				return {
					vk::VertexInputAttributeDescription{ 0, 0, vk::Format::eR32G32Sfloat, offsetof(UIVertex, position) },
					vk::VertexInputAttributeDescription{ 1, 0, vk::Format::eR32G32Sfloat, offsetof(UIVertex,uv) },
					vk::VertexInputAttributeDescription{ 2, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(UIVertex, color) },
				};
			}
		};

		static auto PanelElement(glm::vec2 extent, hd::Texture texture, glm::vec4 color = { 1.f, 1.f, 1.f, 1.f }, glm::vec2 anchor = {0.5f, 0.5f}) -> hd::UIElement;
		static auto TextElement(const std::string& text, float height, hd::CharacterBitmap bitmap, glm::vec4 color = { 0.f, 0.f, 0.f, 1.f }, glm::vec2 anchor = { 0.5f, 0.5f }) -> hd::UIElement;

		auto createVertexBuffers(std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices) -> void;

		auto bind(vk::CommandBuffer& commandBuffer) -> void;
		auto draw(vk::CommandBuffer& commandBuffer) -> void;

	private:
		UIElement() { }

	public:
		hd::CharacterBitmap bitmap; // Be used for both font bitmap or just a picture.

	private:
		uint32_t vertexCount;
		uint32_t indexCount;
		vk::Buffer vertexBuffer;
		vk::DeviceMemory vertexBufferMemory;
		vk::Buffer indexBuffer;
		vk::DeviceMemory indexBufferMemory;
	};
}

