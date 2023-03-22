#include "UIElement.h"

auto Noxg::UIElement::PanelElement(glm::vec2 extent, hd::Texture texture, glm::vec4 color, glm::vec2 anchor) -> hd::UIElement
{
    hd::UIElement element(new UIElement());	// Have to since the default constructor is inaccessible externally.

	std::vector<UIVertex> vertices = {
		UIVertex{ extent * (glm::vec2{ 0.f, 0.f } - anchor ), { 0.f, 1.f }, color },
		UIVertex{ extent * (glm::vec2{ 1.f, 0.f } - anchor ), { 1.f, 1.f }, color },
		UIVertex{ extent * (glm::vec2{ 1.f, 1.f } - anchor ), { 1.f, 0.f }, color },
		UIVertex{ extent * (glm::vec2{ 0.f, 1.f } - anchor ), { 0.f, 0.f }, color },
	};
	std::vector<uint32_t> indices = {
		0, 1, 2,
		2, 3, 0,
	};
	element->createVertexBuffers(vertices, indices);

	element->bitmap = std::make_shared<CharacterBitmap>(texture);

	return element;
}

auto Noxg::UIElement::TextElement(const std::string& text, float height, hd::CharacterBitmap bitmap, glm::vec4 color, glm::vec2 anchor) -> hd::UIElement
{
	hd::UIElement element(new UIElement());	// Have to since the default constructor is inaccessible externally.

	float scale = height / 150.f;

	uint32_t lineCount = 1;
	glm::vec2 textExtent = { 0.f, height };
	std::vector<UIVertex> textVertices;
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
			textExtent.y += height;
			++lineCount;
			continue;
		}

		std::array<UIVertex, 4> charVertices;

		const auto& info = bitmap->charInfos[c];
		glm::vec2 charExtent{ info.x1 - info.x0, info.y1 - info.y0 };

		charVertices[0].position = currentPoint + scale * glm::vec2(info.xoff           , -info.yoff           ); // top left
		charVertices[0].uv = glm::vec2(info.x0, info.y0) / 1024.f;
		charVertices[0].color = color;

		charVertices[1].position = currentPoint + scale * glm::vec2(info.xoff           , -info.yoff - charExtent.y); // bottom left
		charVertices[1].uv = glm::vec2(info.x0, info.y1) / 1024.f;
		charVertices[1].color = color;

		charVertices[2].position = currentPoint + scale * glm::vec2(info.xoff + charExtent.x, -info.yoff - charExtent.y); // bottom right
		charVertices[2].uv = glm::vec2(info.x1, info.y1) / 1024.f;
		charVertices[2].color = color;

		charVertices[3].position = currentPoint + scale * glm::vec2(info.xoff + charExtent.x, -info.yoff           ); // top right
		charVertices[3].uv = glm::vec2(info.x1, info.y0) / 1024.f;
		charVertices[3].color = color;

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
		textExtent.x = glm::max(textExtent.x, currentPoint.x);
	}

	glm::vec2 newOrigin = textExtent * anchor;
	glm::vec2 oldOrigin = { 0.f, height * (lineCount - 1) };
	glm::vec2 offset = oldOrigin - newOrigin;
	for (auto& vertex : textVertices)
	{
		vertex.position += offset;
	}

	element->createVertexBuffers(textVertices, indices);

	element->bitmap = bitmap;

	return element;
}

auto Noxg::UIElement::createVertexBuffers(std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices) -> void
{
	// VertexBuffer
	vertexCount = static_cast<uint32_t>(vertices.size());
	vk::DeviceSize bufferSize = sizeof(UIVertex) * vertexCount;
	uint32_t vertexSize = sizeof(UIVertex);

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
		Utils::CreateBuffer(indexSize, indexCount, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

	Utils::copyBuffer(indexBuffer, stagingBuffer, bufferSize);
	Utils::destroyBuffer(stagingBuffer, stagingBufferMemory);
}

auto Noxg::UIElement::bind(vk::CommandBuffer& commandBuffer) -> void
{
	commandBuffer.bindVertexBuffers(0, { vertexBuffer }, { vk::DeviceSize(0) });
	commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
}

auto Noxg::UIElement::draw(vk::CommandBuffer& commandBuffer) -> void
{
	commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
}
