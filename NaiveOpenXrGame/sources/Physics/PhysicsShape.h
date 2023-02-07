#pragma once

#include "mainCommon.h"
#include "Bricks/GameComponent.h"

namespace Noxg
{
	MAKE_HANDLE(PhysicsShape);

	enum class ShapeType
	{
		Plane,
		Shpere,
		Box,
	};

	class PhysicsShape : public GameComponent
	{
	public:
		PhysicsShape(ShapeType shapeType = ShapeType::Plane);
		PhysicsShape(float raid);
		PhysicsShape(glm::vec3 halfExts);

		virtual void Enable() override;

	public:
		ShapeType type;

		union
		{
			float radius;
			glm::vec3 halfExtents = { 0.5f, 0.5f, 0.5f };
		};

	public:
		PxShape* pxShape = nullptr;
	};
}

