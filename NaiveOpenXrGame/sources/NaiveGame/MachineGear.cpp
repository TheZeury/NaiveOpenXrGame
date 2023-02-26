#include "MachineGear.h"
#include "Renderer/MeshModel.h"
#include "Bricks/GameObject.h"
#include "Physics/PhysicsEngineInstance.h"
#include "Bricks/SceneManager.h"

Noxg::MachineGear::MachineGear(rf::Material _texture, int _level, float _baseSize, float _thickness) : texture{ _texture }, level{ std::max(_level, 1) }, baseSize{ std::abs(_baseSize) }, thickness{ std::abs(_thickness) }
{

}

void Noxg::MachineGear::Enable()
{
	Redraw();
}

void Noxg::MachineGear::Redraw()
{
	float extentLength = level * baseSize;
	float extentDepth = thickness;
	int amount = level * baseAmount;

	std::vector<Vertex> singlePieceVertices = {
		// down
		Vertex{ { -0.5f * extentLength, -0.5f * extentLength, 0.5f * extentDepth }, { }, { }, { 0.f, -1.f, 0.f }, { }, { } },
		Vertex{ { -0.5f * extentLength, -0.5f * extentLength, -0.5f * extentDepth }, { }, { }, { 0.f, -1.f, 0.f }, { }, { } },
		Vertex{ { 0.5f * extentLength, -0.5f * extentLength, -0.5f * extentDepth }, { }, { }, { 0.f, -1.f, 0.f }, { }, { } },
		Vertex{ { 0.5f * extentLength, -0.5f * extentLength, 0.5f * extentDepth }, { }, { }, { 0.f, -1.f, 0.f }, { }, { } },
		// up
		Vertex{ { -0.5f * extentLength, 0.5f * extentLength, 0.5f * extentDepth }, { }, { }, { 0.f, 1.f, 0.f }, { }, { } },
		Vertex{ { 0.5f * extentLength, 0.5f * extentLength, 0.5f * extentDepth }, { }, { }, { 0.f, 1.f, 0.f }, { }, { } },
		Vertex{ { 0.5f * extentLength, 0.5f * extentLength, -0.5f * extentDepth }, { }, { }, { 0.f, 1.f, 0.f }, { }, { } },
		Vertex{ { -0.5f * extentLength, 0.5f * extentLength, -0.5f * extentDepth }, { }, { }, { 0.f, 1.f, 0.f }, { }, { } },
		// front
		Vertex{ { -0.5f * extentLength, -0.5f * extentLength, 0.5f * extentDepth }, { }, { }, { 0.f, 0.f, 1.f }, { }, { } },
		Vertex{ { 0.5f * extentLength, -0.5f * extentLength, 0.5f * extentDepth }, { }, { }, { 0.f, 0.f, 1.f }, { }, { } },
		Vertex{ { 0.5f * extentLength, 0.5f * extentLength, 0.5f * extentDepth }, { }, { }, { 0.f, 0.f, 1.f }, { }, { } },
		Vertex{ { -0.5f * extentLength, 0.5f * extentLength, 0.5f * extentDepth }, { }, { }, { 0.f, 0.f, 1.f }, { }, { } },
		// back
		Vertex{ { -0.5f * extentLength, -0.5f * extentLength, -0.5f * extentDepth }, { }, { }, { 0.f, 0.f, -1.f }, { }, { } },
		Vertex{ { -0.5f * extentLength, 0.5f * extentLength, -0.5f * extentDepth }, { }, { }, { 0.f, 0.f, -1.f }, { }, { } },
		Vertex{ { 0.5f * extentLength, 0.5f * extentLength, -0.5f * extentDepth }, { }, { }, { 0.f, 0.f, -1.f }, { }, { } },
		Vertex{ { 0.5f * extentLength, -0.5f * extentLength, -0.5f * extentDepth }, { }, { }, { 0.f, 0.f, -1.f }, { }, { } },
		// left
		Vertex{ { -0.5f * extentLength, -0.5f * extentLength, 0.5f * extentDepth }, { }, { }, { -1.f, 0.f, 0.f }, { }, { } },
		Vertex{ { -0.5f * extentLength, 0.5f * extentLength, 0.5f * extentDepth }, { }, { }, { -1.f, 0.f, 0.f }, { }, { } },
		Vertex{ { -0.5f * extentLength, 0.5f * extentLength, -0.5f * extentDepth }, { }, { }, { -1.f, 0.f, 0.f }, { }, { } },
		Vertex{ { -0.5f * extentLength, -0.5f * extentLength, -0.5f * extentDepth }, { }, { }, { -1.f, 0.f, 0.f }, { }, { } },
		// right
		Vertex{ { 0.5f * extentLength, -0.5f * extentLength, 0.5f * extentDepth }, { }, { }, { 1.f, 0.f, 0.f }, { }, { } },
		Vertex{ { 0.5f * extentLength, -0.5f * extentLength, -0.5f * extentDepth }, { }, { }, { 1.f, 0.f, 0.f }, { }, { } },
		Vertex{ { 0.5f * extentLength, 0.5f * extentLength, -0.5f * extentDepth }, { }, { }, { 1.f, 0.f, 0.f }, { }, { } },
		Vertex{ { 0.5f * extentLength, 0.5f * extentLength, 0.5f * extentDepth }, { }, { }, { 1.f, 0.f, 0.f }, { }, { } },
	};
	std::vector<uint32_t> singlePieceIndices = {
		0 + 0,  1 + 0,  2 + 0,	// down
		2 + 0,  3 + 0,  0 + 0,
		0 + 4,  1 + 4,  2 + 4,	// up
		2 + 4,  3 + 4,  0 + 4,
		0 + 8,  1 + 8,  2 + 8,	// front
		2 + 8,  3 + 8,  0 + 8,
		0 + 12, 1 + 12, 2 + 12,	// back
		2 + 12, 3 + 12, 0 + 12,
		0 + 16, 1 + 16, 2 + 16,	// left
		2 + 16, 3 + 16, 0 + 16,
		0 + 20, 1 + 20, 2 + 20,	// right
		2 + 20, 3 + 20, 0 + 20,
	};

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (int i = 0; i < amount; ++i)
	{
		float angle = glm::pi<float>() * 0.5f * float(i) / amount;
		glm::quat rotation = glm::rotate(glm::quat{ 1.f, 0.f, 0.f, 0.f }, angle, { 0.f, 0.f, 1.f });
		size_t vertexOffset = vertices.size();
		size_t indexOffset = indices.size();
		for (const auto& vertex : singlePieceVertices)
		{
			Vertex newVertex{
				rotation * vertex.position,
				vertex.color,
				vertex.uv,
				rotation * vertex.normal,
				rotation * vertex.tangent,
				rotation * vertex.bitangent,
			};
			vertices.push_back(newVertex);
		}
		for (const auto& index : singlePieceIndices)
		{
			indices.push_back(static_cast<uint32_t>(vertexOffset) + index);
		}
	}
	
	gameObject.lock()->models.clear();
	gameObject.lock()->models.push_back(std::make_shared<MeshModel>(vertices, indices, texture.lock()));
}

std::vector<PxShape*> Noxg::MachineGear::getRecommendedColliders()
{
	hd::PhysicsEngineInstance physics = gameObject.lock()->scene.lock()->manager.lock()->defaultPhysicsEngineInstance.lock();
	std::vector<PxShape*> shapes;
	PxBoxGeometry box(level * baseSize * 0.5f, level * baseSize * 0.5f, thickness * 0.5f);
	PxShape* shape = physics->createShape(box);
	shapes.push_back(shape);
	shape = physics->createShape(box);
	glm::quat rotation = glm::rotate(glm::quat{ 1.f, 0.f, 0.f, 0.f }, glm::pi<float>() * 0.5f, { 0.f, 0.f, 1.f });
	shape->setLocalPose(PxTransform(*(PxQuat*)(&rotation)));
	shapes.push_back(shape);
	return shapes;
}

void Noxg::MachineGear::setLevel(int newLevel)
{
	level = std::max(newLevel, 1);
	Redraw();
}

void Noxg::MachineGear::setBaseSize(float newBaseSize)
{
	baseSize = std::abs(newBaseSize);
	Redraw();
}
