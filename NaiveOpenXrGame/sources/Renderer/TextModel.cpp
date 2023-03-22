#include "TextModel.h"

Noxg::TextModel::TextModel(const std::string& text, hd::Material material, hd::CharacterBitmap bitmap, float height) : material{ material }, bitmap{ bitmap }
{
	createTextModel(text, height);
}

void Noxg::TextModel::createTextModel(const std::string& text, float height)
{
	float scale = height / 150.f;

	std::vector<TextVertex> textVertices;
	std::vector<uint32_t> indices;

	uint32_t index = 0;

	glm::vec2 currentPoint{ };
	for (auto c : text)
	{
		index = static_cast<uint32_t>(textVertices.size());

		if (c == '\n')
		{
			currentPoint.x = 0.f;
			currentPoint.y -= height;
			continue;
		}

		std::array<TextVertex, 4> charVertices = {
			TextVertex{ { }, { 0.f, 0.f }, { }, {  0.f,  0.f, -1.f }, { }, { } },
			TextVertex{ { }, { 0.f, 1.f }, { }, {  0.f,  0.f, -1.f }, { }, { } },
			TextVertex{ { }, { 1.f, 1.f }, { }, {  0.f,  0.f, -1.f }, { }, { } },
			TextVertex{ { }, { 1.f, 0.f }, { }, {  0.f,  0.f, -1.f }, { }, { } },
		};

		const auto& info = bitmap->charInfos[c];
		glm::vec2 extent{ info.x1 - info.x0, info.y1 - info.y0 };
		
		charVertices[0].position = glm::vec3(currentPoint + scale * glm::vec2(info.xoff           , -info.yoff           ), 0.f); // top left
		charVertices[0].bitmapUv = glm::vec2(info.x0, info.y0) / 1024.f;

		charVertices[1].position = glm::vec3(currentPoint + scale * glm::vec2(info.xoff           , -info.yoff - extent.y), 0.f); // bottom left
		charVertices[1].bitmapUv = glm::vec2(info.x0, info.y1) / 1024.f;

		charVertices[2].position = glm::vec3(currentPoint + scale * glm::vec2(info.xoff + extent.x, -info.yoff - extent.y), 0.f); // bottom right
		charVertices[2].bitmapUv = glm::vec2(info.x1, info.y1) / 1024.f;

		charVertices[3].position = glm::vec3(currentPoint + scale * glm::vec2(info.xoff + extent.x, -info.yoff           ), 0.f); // top right
		charVertices[3].bitmapUv = glm::vec2(info.x1, info.y0) / 1024.f;

		for (auto& v : charVertices)
		{
			textVertices.push_back(v);
		}

		indices.push_back(index + 0);
		indices.push_back(index + 1);
		indices.push_back(index + 2);


		indices.push_back(index + 2);
		indices.push_back(index + 3);
		indices.push_back(index + 0);

		currentPoint.x += scale * info.xadvance;
	}

	// VertexBuffer
	vertexCount = static_cast<uint32_t>(textVertices.size());
	vk::DeviceSize bufferSize = sizeof(TextVertex) * vertexCount;
	uint32_t vertexSize = sizeof(TextVertex);

	auto [stagingBuffer, stagingBufferMemory] =
		Utils::CreateBuffer(vertexSize, vertexCount, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	void* mappingMemory = Utils::mapMemory(stagingBufferMemory);
	std::memcpy(mappingMemory, textVertices.data(), bufferSize);
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
		Utils::CreateBuffer(indexSize, indexCount, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

	Utils::copyBuffer(indexBuffer, stagingBuffer, bufferSize);
	Utils::destroyBuffer(stagingBuffer, stagingBufferMemory);
}

void Noxg::TextModel::bind(vk::CommandBuffer& commandBuffer)
{
	commandBuffer.bindVertexBuffers(0, { vertexBuffer }, { vk::DeviceSize(0) });
	commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
}

void Noxg::TextModel::draw(vk::CommandBuffer& commandBuffer)
{
	commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
}
