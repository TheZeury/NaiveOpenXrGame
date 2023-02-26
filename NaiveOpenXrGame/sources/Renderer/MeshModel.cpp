#include "MeshModel.h"
#include "Utils.h"

Noxg::MeshModel::MeshModel(std::string path, hd::Material tex)
{
	material = tex;
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), "models"))
	{
		throw std::runtime_error(warn + err);
	}

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::unordered_map<Vertex, uint32_t> uniqueVertices;

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex{ };
			
			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2],
			};

			vertex.normal = {
				attrib.normals[3 * index.vertex_index + 0],
				attrib.normals[3 * index.vertex_index + 1],
				attrib.normals[3 * index.vertex_index + 2],
			};

			vertex.uv = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.f - attrib.texcoords[2 * index.texcoord_index + 1],
			};

			vertex.color = {
				1.f, 1.f, 1.f, 1.f
			};

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}

	createMeshModel(vertices, indices);
}

Noxg::MeshModel::MeshModel(std::vector<Vertex> vertices, std::vector<uint32_t> indices, hd::Material tex)
{
	material = tex;
	createMeshModel(vertices, indices);
}

Noxg::MeshModel::~MeshModel()
{
	Utils::destroyBuffer(vertexBuffer, vertexBufferMemory);
	Utils::destroyBuffer(indexBuffer, indexBufferMemory);
}

void Noxg::MeshModel::createMeshModel(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, bool calculateTBN)
{
	if (calculateTBN)
	{
		calculateTangentBitangent(vertices, indices);
	}
	// VertexBuffer
	vertexCount = static_cast<uint32_t>(vertices.size());
	vk::DeviceSize bufferSize = sizeof(Vertex) * vertexCount;
	uint32_t vertexSize = sizeof(Vertex);

	auto [stagingBuffer, stagingBufferMemory] = 
		Utils::CreateBuffer(vertexSize, vertexCount, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	void* mappingMemory = Utils::mapMemory(stagingBufferMemory);
	std::memcpy(mappingMemory, vertices.data(), bufferSize);
	Utils::unmapMemory(stagingBufferMemory);

	std::tie(vertexBuffer, vertexBufferMemory) =
		Utils::CreateBuffer(vertexSize, vertexCount, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	Utils::copyBuffer(vertexBuffer, stagingBuffer, bufferSize);
	Utils::destroyBuffer(stagingBuffer, stagingBufferMemory);

	// VertexBuffer
	indexCount = static_cast<uint32_t>(indices.size());
	bufferSize = sizeof(indices[0]) * indexCount;
	uint32_t indexSize = sizeof(indices[0]);

	std::tie(stagingBuffer, stagingBufferMemory) = 
		Utils::CreateBuffer(indexSize, indexCount, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	mappingMemory = Utils::mapMemory(stagingBufferMemory);
	std::memcpy(mappingMemory, indices.data(), bufferSize);
	Utils::unmapMemory(stagingBufferMemory);

	std::tie(indexBuffer, indexBufferMemory) = 
		Utils::CreateBuffer(vertexSize, vertexCount, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

	Utils::copyBuffer(indexBuffer, stagingBuffer, bufferSize);
	Utils::destroyBuffer(stagingBuffer, stagingBufferMemory);
}

void Noxg::MeshModel::bind(vk::CommandBuffer& commandBuffer)
{
	std::array<vk::Buffer, 1> vertexBuffers = { vertexBuffer };
	std::array<vk::DeviceSize, 1> offsets = { 0 };
	commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);

	commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
}

void Noxg::MeshModel::draw(vk::CommandBuffer& commandBuffer)
{
	commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
}

void Noxg::MeshModel::calculateTangentBitangent(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	if (indices.size() % 3 != 0)
	{
		throw std::runtime_error("Not a complete triangle mesh.");
	}
	std::vector<uint32_t> count(vertices.size(), 0);

	for (size_t i = 0; i < indices.size(); i += 3)
	{
		uint32_t index0 = indices[i + 0];
		uint32_t index1 = indices[i + 1];
		uint32_t index2 = indices[i + 2];

		++count[index0];
		++count[index1];
		++count[index2];

		Vertex& v0 = vertices[index0];
		Vertex& v1 = vertices[index1];
		Vertex& v2 = vertices[index2];

		glm::vec3& p0 = v0.position;
		glm::vec3& p1 = v1.position;
		glm::vec3& p2 = v2.position;

		glm::vec2& u0 = v0.uv;
		glm::vec2& u1 = v1.uv;
		glm::vec2& u2 = v2.uv;

		glm::vec3 deltaP1 = p1 - p0;
		glm::vec3 deltaP2 = p2 - p0;

		glm::vec2 deltaU1 = u1 - u0;
		glm::vec2 deltaU2 = u2 - u0;

		float r = 1.0f / (deltaU1.x * deltaU2.y - deltaU2.x * deltaU1.y);
		glm::vec3 tangent = (deltaP1 * deltaU2.y - deltaP2 * deltaU1.y) * r;
		glm::vec3 bitangent = (deltaP2 * deltaU1.x - deltaP1 * deltaU2.x) * r;

		v0.tangent += tangent;
		v0.bitangent += bitangent;

		v1.tangent += tangent;
		v1.bitangent += bitangent;

		v2.tangent += tangent;
		v2.bitangent += bitangent;
	}

	for (int i = 0; i < vertices.size(); ++i)
	{
		vertices[i].tangent /= count[i];
		vertices[i].bitangent /= count[i];
	}
}
