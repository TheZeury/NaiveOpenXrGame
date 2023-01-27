#pragma once

#include "mainCommon.h"

namespace Noxg
{
	class MeshModel
	{
	public:
		struct Vertex
		{
			glm::vec3 position;
			glm::vec4 color;
			glm::vec2 uv;

			static std::array<vk::VertexInputBindingDescription, 1> getBindingDescriptions()
			{
				return {
					vk::VertexInputBindingDescription{ 0, sizeof(Vertex), vk::VertexInputRate::eVertex },
				};
			}

			static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions()
			{
				return {
					vk::VertexInputAttributeDescription{ 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position) },
					vk::VertexInputAttributeDescription{ 1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color) },
					vk::VertexInputAttributeDescription{ 2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv) },
				};
			}

			bool operator==(const Vertex& other) const
			{
				return position == other.position && color == other.color && uv == other.uv;
			}
		};
	public:
		MeshModel(const MeshModel&) = delete;
		MeshModel& operator=(const MeshModel&) = delete;

		MeshModel(std::string path);
		MeshModel(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
		~MeshModel();

		void createMeshModel(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

		void bind(vk::CommandBuffer& commandBuffer);
		void draw(vk::CommandBuffer& commandBuffer);
	private:
		uint32_t vertexCount;
		uint32_t indexCount;
		vk::Buffer vertexBuffer;
		vk::DeviceMemory vertexBufferMemory;
		vk::Buffer indexBuffer;
		vk::DeviceMemory indexBufferMemory;
	};
}