#include "MeshModel.h"
#include "Utils.h"

Noxg::MeshModel::MeshModel(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
{
	// VertexBuffer
	vertexCount = static_cast<uint32_t>(vertices.size());
	vk::DeviceSize bufferSize = sizeof(Vertex) * vertexCount;
	uint32_t vertexSize = sizeof(Vertex);

	auto [stagingBuffer, stagingBufferMemory] = 
		Utils::AllocateBuffer(vertexSize, vertexCount, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	void* mappingMemory = Utils::mapMemory(stagingBufferMemory);
	std::memcpy(mappingMemory, vertices.data(), bufferSize);
	Utils::unmapMemory(stagingBufferMemory);

	vertexBuffer = stagingBuffer;
	vertexBufferMemory = stagingBufferMemory;

	/*std::tie(vertexBuffer, vertexBufferMemory) =
		Utils::AllocateBuffer(vertexSize, vertexCount, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	Utils::copyBuffer(vertexBuffer, stagingBuffer, bufferSize);
	Utils::destroyBuffer(stagingBuffer, stagingBufferMemory);*/

	// VertexBuffer
	indexCount = indices.size();
	bufferSize = sizeof(indices[0]) * indexCount;
	uint32_t indexSize = sizeof(indices[0]);

	std::tie(stagingBuffer, stagingBufferMemory) = 
		Utils::AllocateBuffer(indexSize, indexCount, vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	mappingMemory = Utils::mapMemory(stagingBufferMemory);
	std::memcpy(mappingMemory, indices.data(), bufferSize);
	Utils::unmapMemory(stagingBufferMemory);

	indexBuffer = stagingBuffer;
	indexBufferMemory = stagingBufferMemory;

	/*std::tie(indexBuffer, indexBufferMemory) = 
		Utils::AllocateBuffer(vertexSize, vertexCount, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

	Utils::copyBuffer(indexBuffer, stagingBuffer, bufferSize);
	Utils::destroyBuffer(stagingBuffer, stagingBufferMemory);*/
}

Noxg::MeshModel::~MeshModel()
{
	Utils::destroyBuffer(vertexBuffer, vertexBufferMemory);
	Utils::destroyBuffer(indexBuffer, indexBufferMemory);
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
