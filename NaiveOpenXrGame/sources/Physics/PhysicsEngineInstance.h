#pragma once

#include "mainCommon.h"
#include "Bricks/Scene.h"
#include <list>

namespace Noxg
{
	MAKE_HANDLE(PhysicsEngineInstance);

	class PhysicsEngineInstance
	{
	public:
		virtual void Initialize() = 0;
		virtual void Simulate(float timeDelta) = 0;
		virtual void addScene(rf::Scene scene) = 0;
		virtual PxScene* createScene() const = 0;
		virtual PxRigidDynamic* createRigidDynamic(const PxTransform& pose) const = 0;
		virtual PxShape* createShape(const PxGeometry& geometry) const = 0;
	protected:
		std::list<rf::Scene> scenes;
	};
}

