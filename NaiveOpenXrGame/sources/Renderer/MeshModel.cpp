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
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2],
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

uint32_t Noxg::MeshBuilder::addVertex(Vertex vertex)
{
	if (!uniqueVertices.contains(vertex))
	{
		if(removedVertices.empty())
		{
			uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
			vertices.push_back(vertex);
		}
		else
		{
			auto index = *removedVertices.begin(); removedVertices.erase(index);
			uniqueVertices[vertex] = index;
			vertices[index] = vertex;
		}
	}
	return uniqueVertices[vertex];
}

uint32_t Noxg::MeshBuilder::addTriangle(uint32_t v1, uint32_t v2, uint32_t v3)
{
	if (v1 == v2 || v2 == v3 || v3 == v1) 
	{
		throw std::runtime_error("Three vertices must be all different.");
	}
	auto tester = [&](uint32_t v) { if (v >= vertices.size() || removedVertices.contains(v)) throw std::runtime_error(std::format("Newly added vertex {} doesn't exist.", v)); };
	tester(v1); tester(v2); tester(v3);

	IndexedTriangle triangle;

	if (v1 < v2 && v1 < v3) triangle = std::make_tuple(v1, v2, v3);		// deduplication.
	else if (v2 < v3) triangle = std::make_tuple(v2, v3, v1);			//
	else triangle = std::make_tuple(v3, v1, v2);						//

	if (!uniqueTriangles.contains(triangle))
	{
		if(removeTriangles.empty())
		{
			uniqueTriangles[triangle] = static_cast<uint32_t>(triangles.size());
			triangles.push_back(triangle);
		}
		else
		{
			auto index = *removeTriangles.begin(); removeTriangles.erase(index);
			uniqueTriangles[triangle] = index;
			triangles[index] = triangle;
		}
	}
	return uniqueTriangles[triangle];
}

std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> Noxg::MeshBuilder::addTriangle(Vertex v1, Vertex v2, Vertex v3)
{
	auto i1 = addVertex(v1), i2 = addVertex(v2), i3 = addVertex(v3);
	return std::make_tuple(addTriangle(i1, i2, i3), i1, i2, i3);
}

void Noxg::MeshBuilder::updateVertex(uint32_t v, Vertex value)
{
	uniqueVertices.erase(vertices[v]);
	vertices[v] = value;
	uniqueVertices[value] = v;
}

void Noxg::MeshBuilder::removeVertex(uint32_t v)
{
	// TODO
	uniqueVertices.erase(vertices[v]);
	removedVertices.insert(v);
	
	for (auto [triangle, index] : uniqueTriangles)
	{
		auto [v1, v2, v3] = triangle;
		if (v1 == v || v2 == v || v3 == v)
		{
			removeTriangle(index);
		}
	}
}

void Noxg::MeshBuilder::removeTriangle(uint32_t t)
{
	uniqueTriangles.erase(triangles[t]);
	removeTriangles.insert(t);
}

Noxg::Vertex Noxg::MeshBuilder::getVertex(uint32_t v)
{
	return vertices[v];
}

Noxg::Triangle Noxg::MeshBuilder::getTriangle(uint32_t t)
{
	auto [v1, v2, v3] = triangles[t];
	return std::make_tuple(vertices[v1], vertices[v2], vertices[v3]);
}

Noxg::IndexedTriangle Noxg::MeshBuilder::getIndexedTriangle(uint32_t t)
{
	return triangles[t];
}

Noxg:: hd::MeshModel Noxg::MeshBuilder::build(hd::Material material)
{
	std::vector<Vertex> buildVertices;
	std::vector<uint32_t> buildIndices;
	std::vector<uint32_t> vertices_to_buildVertices_map(static_cast<size_t>(vertices.size()));

	for (uint32_t v = 0; v < vertices.size(); ++v)
	{
		vertices_to_buildVertices_map[v] = static_cast<uint32_t>(buildVertices.size());
		if (!removedVertices.contains(v))
		{
			buildVertices.push_back(vertices[v]);
		}
	}

	for (auto& triangle : triangles)
	{
		auto [v1, v2, v3] = triangle;
		buildIndices.push_back(vertices_to_buildVertices_map[v1]);
		buildIndices.push_back(vertices_to_buildVertices_map[v2]);
		buildIndices.push_back(vertices_to_buildVertices_map[v3]);
	}
	return std::make_shared<MeshModel>(buildVertices, buildIndices, material);
}

Noxg::MeshBuilder Noxg::MeshBuilder::Box(float halfX, float halfY, float halfZ)
{
	MeshBuilder mesh;
	mesh.vertices = {
		// down
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{ -1.f, -1.f,  1.f }, { }, { }, {  0.f, -1.f,  0.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{ -1.f, -1.f, -1.f }, { }, { }, {  0.f, -1.f,  0.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{  1.f, -1.f, -1.f }, { }, { }, {  0.f, -1.f,  0.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{  1.f, -1.f,  1.f }, { }, { }, {  0.f, -1.f,  0.f }, { }, { } },
		// up
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{ -1.f,  1.f,  1.f }, { }, { }, {  0.f,  1.f,  0.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{  1.f,  1.f,  1.f }, { }, { }, {  0.f,  1.f,  0.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{  1.f,  1.f, -1.f }, { }, { }, {  0.f,  1.f,  0.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{ -1.f,  1.f, -1.f }, { }, { }, {  0.f,  1.f,  0.f }, { }, { } },
		// front
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{ -1.f, -1.f,  1.f }, { }, { }, {  0.f,  0.f,  1.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{  1.f, -1.f,  1.f }, { }, { }, {  0.f,  0.f,  1.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{  1.f,  1.f,  1.f }, { }, { }, {  0.f,  0.f,  1.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{ -1.f,  1.f,  1.f }, { }, { }, {  0.f,  0.f,  1.f }, { }, { } },
		// back
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{ -1.f, -1.f, -1.f }, { }, { }, {  0.f,  0.f, -1.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{ -1.f,  1.f, -1.f }, { }, { }, {  0.f,  0.f, -1.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{  1.f,  1.f, -1.f }, { }, { }, {  0.f,  0.f, -1.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{  1.f, -1.f, -1.f }, { }, { }, {  0.f,  0.f, -1.f }, { }, { } },
		// left
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{ -1.f, -1.f,  1.f }, { }, { }, { -1.f,  0.f,  0.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{ -1.f,  1.f,  1.f }, { }, { }, { -1.f,  0.f,  0.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{ -1.f,  1.f, -1.f }, { }, { }, { -1.f,  0.f,  0.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{ -1.f, -1.f, -1.f }, { }, { }, { -1.f,  0.f,  0.f }, { }, { } },
		// right
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{  1.f, -1.f,  1.f }, { }, { }, {  1.f,  0.f,  0.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{  1.f, -1.f, -1.f }, { }, { }, {  1.f,  0.f,  0.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{  1.f,  1.f, -1.f }, { }, { }, {  1.f,  0.f,  0.f }, { }, { } },
		Vertex{ glm::vec3{ halfX, halfY, halfZ } * glm::vec3{  1.f,  1.f,  1.f }, { }, { }, {  1.f,  0.f,  0.f }, { }, { } },
	};
	mesh.triangles = {
		IndexedTriangle{ 0 +  0, 1 +  0, 2 +  0, },	// down
		IndexedTriangle{ 2 +  0, 3 +  0, 0 +  0, },
		IndexedTriangle{ 0 +  4, 1 +  4, 2 +  4, },	// up
		IndexedTriangle{ 2 +  4, 3 +  4, 0 +  4, },
		IndexedTriangle{ 0 +  8, 1 +  8, 2 +  8, },	// front
		IndexedTriangle{ 2 +  8, 3 +  8, 0 +  8, },
		IndexedTriangle{ 0 + 12, 1 + 12, 2 + 12, },	// back
		IndexedTriangle{ 2 + 12, 3 + 12, 0 + 12, },
		IndexedTriangle{ 0 + 16, 1 + 16, 2 + 16, },	// left
		IndexedTriangle{ 2 + 16, 3 + 16, 0 + 16, },
		IndexedTriangle{ 0 + 20, 1 + 20, 2 + 20, },	// right
		IndexedTriangle{ 2 + 20, 3 + 20, 0 + 20, },
	};
	return mesh;
}

Noxg::MeshBuilder Noxg::MeshBuilder::UVSphere(float radius, uint32_t rings, uint32_t segments)
{
	MeshBuilder mesh;

	if (rings < 2) rings = 2;
	if (segments < 3) segments = 3;

	auto v0 = mesh.addVertex(Vertex{ { 0.f, radius, 0.f }, { }, { }, { 0.f,  1.f, 0.f }, { }, { } });

	for (uint32_t i = 0; i < rings - 1; ++i)
	{
		auto phi = glm::pi<float>() * (i + 1) / rings;
		for (uint32_t j = 0; j < segments; ++j)
		{
			auto theta = 2.f * glm::pi<float>() * j / segments;
			auto x = glm::sin(phi) * glm::cos(theta);
			auto y = glm::cos(phi);
			auto z = glm::sin(phi) * glm::sin(theta);
			mesh.addVertex(Vertex{ radius * glm::vec3{ x, y, z }, { }, { }, glm::normalize(glm::vec3{ x, y, z }), { }, { } });
		}
	}

	auto v1 = mesh.addVertex(Vertex{ { 0.f, -radius, 0.f }, { }, { }, { 0.f, -1.f, 0.f }, { }, { } });

	for (uint32_t i = 0; i < segments; ++i)
	{
		auto i0 =                  i + 1;
		auto i1 = (i + 1) % segments + 1;
		mesh.addTriangle(v0, i1, i0);
		i0 =                  i + segments * (rings - 2) + 1;
		i1 = (i + 1) % segments + segments * (rings - 2) + 1;
		mesh.addTriangle(v1, i0, i1);
	}

	for (uint32_t j = 0; j < rings - 2; ++j)
	{
		auto j0 =       j * segments + 1;
		auto j1 = (j + 1) * segments + 1;
		for (uint32_t i = 0; i < segments; ++i)
		{
			auto i0 = j0 +                  i;
			auto i1 = j0 + (i + 1) % segments;
			auto i2 = j1 + (i + 1) % segments;
			auto i3 = j1 +                  i;
			mesh.addTriangle(i0, i1, i2);
			mesh.addTriangle(i2, i3, i0);
		}
	}

	return mesh;
}
