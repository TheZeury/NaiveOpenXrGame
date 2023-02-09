#pragma once

#include "mainCommon.h"
#include "Texture.h"

namespace Noxg
{
	MAKE_HANDLE(MeshModel);

	class MeshModel
	{
	public:
		struct Vertex
		{
			glm::vec3 position;
			glm::vec4 color;
			glm::vec3 normal;
			glm::vec2 uv;

			static std::array<vk::VertexInputBindingDescription, 1> getBindingDescriptions()
			{
				return {
					vk::VertexInputBindingDescription{ 0, sizeof(Vertex), vk::VertexInputRate::eVertex },
				};
			}

			static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions()
			{
				return {
					vk::VertexInputAttributeDescription{ 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position) },
					vk::VertexInputAttributeDescription{ 1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color) },
					vk::VertexInputAttributeDescription{ 2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal) },
					vk::VertexInputAttributeDescription{ 3, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv) },
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

		MeshModel(std::string path, hd::Texture tex);
		MeshModel(std::vector<Vertex> vertices, std::vector<uint32_t> indices, hd::Texture tex);
		~MeshModel();

		void createMeshModel(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

		void bind(vk::CommandBuffer& commandBuffer);
		void draw(vk::CommandBuffer& commandBuffer);
	public:
		hd::Texture texture;

	private:
		uint32_t vertexCount;
		uint32_t indexCount;
		vk::Buffer vertexBuffer;
		vk::DeviceMemory vertexBufferMemory;
		vk::Buffer indexBuffer;
		vk::DeviceMemory indexBufferMemory;
	};

	using Vertex = MeshModel::Vertex;
}


namespace std
{
	template<>
	struct hash<Noxg::MeshModel::Vertex>
	{
		size_t operator()(Noxg::MeshModel::Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec4>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.uv) << 1);
		}
	};
}