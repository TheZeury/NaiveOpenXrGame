#pragma once

#include "mainCommon.h"
#include "Material.h"
#include "IModel.h"

namespace Noxg
{
	MAKE_HANDLE(MeshModel);

	class MeshModel : public IModel
	{
	public:
		struct Vertex
		{
			glm::vec3 position;
			glm::vec4 color;
			glm::vec2 uv;
			glm::vec3 normal;
			glm::vec3 tangent;
			glm::vec3 bitangent;

			static std::array<vk::VertexInputBindingDescription, 1> getBindingDescriptions()
			{
				return {
					vk::VertexInputBindingDescription{ 0, sizeof(Vertex), vk::VertexInputRate::eVertex },
				};
			}

			static std::array<vk::VertexInputAttributeDescription, 6> getAttributeDescriptions()
			{
				return {
					vk::VertexInputAttributeDescription{ 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position) },
					vk::VertexInputAttributeDescription{ 1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color) },
					vk::VertexInputAttributeDescription{ 2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv) },
					vk::VertexInputAttributeDescription{ 3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal) },
					vk::VertexInputAttributeDescription{ 4, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, tangent) },
					vk::VertexInputAttributeDescription{ 5, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, bitangent) },
				};
			}

			bool operator==(const Vertex& other) const
			{
				return position == other.position && normal == other.normal && uv == other.uv;	
				// There's no need to take tangent and bitangent into consideration since they are averaged through all vertices that have the same position, uv, and normal values;
			}
		};
	public:
		MeshModel(const MeshModel&) = delete;
		MeshModel& operator=(const MeshModel&) = delete;

		MeshModel(std::string path, hd::Material tex);
		MeshModel(std::vector<Vertex> vertices, std::vector<uint32_t> indices, hd::Material tex);
		~MeshModel();

		void createMeshModel(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, bool calculateTBN = true);

		void bind(vk::CommandBuffer& commandBuffer);
		void draw(vk::CommandBuffer& commandBuffer);

		static void calculateTangentBitangent(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	public:
		hd::Material material;

	private:
		uint32_t vertexCount;
		uint32_t indexCount;
		vk::Buffer vertexBuffer;
		vk::DeviceMemory vertexBufferMemory;
		vk::Buffer indexBuffer;
		vk::DeviceMemory indexBufferMemory;
	};

	using Vertex = MeshModel::Vertex;

	using IndexedTriangle = std::tuple<uint32_t, uint32_t, uint32_t>;
	using Triangle = std::tuple<Vertex, Vertex, Vertex>;
}


namespace std
{
	template<>
	struct hash<Noxg::MeshModel::Vertex>
	{
		size_t operator()(Noxg::MeshModel::Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.uv) << 1);
		}
	};

	template<>
	struct hash<Noxg::IndexedTriangle>
	{
		size_t operator()(Noxg::IndexedTriangle const& triangle) const
		{
			auto [v1, v2, v3] = triangle;
			return ((hash<uint32_t>()(v1) ^
				(hash<uint32_t>()(v2) << 1)) >> 1) ^
				(hash<uint32_t>()(v3) << 1);
		}
	};
}

namespace Noxg
{
	class MeshBuilder
	{
	public:
		// Add a vertex and return it's index in the model.
		uint32_t addVertex(Vertex vertex);

		// Add a new Traingle based on three vertices. Counter-clock side is the front. Face index is returned.
		uint32_t addTriangle(uint32_t v1, uint32_t v2, uint32_t v3);

		// Add a new Traingle with three specified vertices. return a tuple of [traingle_index, vertex_index_1, vertex_index_2, vertex_index_3];
		std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> addTriangle(Vertex v1, Vertex v2, Vertex v3);
		
		// Update a existing vertex by pass it's index and new value.
		void updateVertex(uint32_t v, Vertex value);
		
		// Remove a particular vertex. All traingles using this vertex will be deleted also. Avoid calling this since current implementaton is expensive ( Iterate through all face to find those to delete. ).
		void removeVertex(uint32_t v);
		
		// Remove a particular traingle. Vertics it's using won't be deleted.
		void removeTriangle(uint32_t t);

		// 
		Vertex getVertex(uint32_t v);

		// 
		Triangle getTriangle(uint32_t t);

		//
		IndexedTriangle getIndexedTriangle(uint32_t t);

		// Build a mesh model.
		hd::MeshModel build(hd::Material material);

	public:
		static MeshBuilder Box(float halfX, float halfY, float halfZ);
		static MeshBuilder UVSphere(float radius, uint32_t rings, uint32_t segments);
		static MeshBuilder Icosphere(float radius, uint32_t level);
		static MeshBuilder Cone(float bottomRadius, float topRadius, float height, uint32_t segments);

	private:
		std::vector<Vertex> vertices;
		std::vector<IndexedTriangle> triangles;
		std::unordered_map<Vertex, uint32_t> uniqueVertices;
		std::unordered_map<IndexedTriangle, uint32_t> uniqueTriangles;
		std::unordered_set<uint32_t> removedVertices;
		std::unordered_set<uint32_t> removeTriangles;
	};
}