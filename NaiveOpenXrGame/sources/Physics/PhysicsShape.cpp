#include "PhysicsShape.h"
#include "Bricks/GameObject.h"
#include "Bricks/Scene.h"
#include "Bricks/SceneManager.h"

Noxg::PhysicsShape::PhysicsShape(ShapeType shapeType) : type{ shapeType }
{
}

Noxg::PhysicsShape::PhysicsShape(float raid) : type{ ShapeType::Shpere }, radius { raid }
{
}

Noxg::PhysicsShape::PhysicsShape(glm::vec3 halfExts) : type{ ShapeType::Box }, halfExtents{ halfExts }
{
}

void Noxg::PhysicsShape::Enable()
{
	auto obj = gameObject.lock();
	auto physicsEngineInstance = obj->scene.lock()->manager.lock()->defaultPhysicsEngineInstance;
	auto physics = physicsEngineInstance.lock();

	switch (type)
	{
	case Noxg::ShapeType::Plane: {
		pxShape = physics->createShape(PxPlaneGeometry());
	}
		break;
	case Noxg::ShapeType::Shpere: {
		pxShape = physics->createShape(PxSphereGeometry(radius));
	}
		break;
	case Noxg::ShapeType::Box: {
		pxShape = physics->createShape(PxBoxGeometry(*((PxVec3*)(&halfExtents))));
	}
		break;
	default: {
		throw std::runtime_error("Unknown physics shape type.");
	}
		break;
	}
}
