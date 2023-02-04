#pragma once

#include "mainCommon.h"
#include "Physics/PhysicsEngineInstance.h"
#include "PhysicsTransform.h"
#include "Bricks/GameObject.h"

namespace Noxg
{
	MAKE_HANDLE(RigidDynamic);

	class RigidDynamic
	{
	public:
		RigidDynamic(rf::GameObject obj, rf::PhysicsEngineInstance physicsInstance, PxScene* physicsScene, glm::vec3 vec = { }, PxForceMode::Enum forceMode = PxForceMode::eVELOCITY_CHANGE);
		~RigidDynamic();
		
	public:
		rf::PhysicsEngineInstance physicsEngineInstance;
		rf::GameObject gameObject;
		glm::vec3 force;
		PxForceMode::Enum mode;
	private:
		PxRigidDynamic* pxRaw = nullptr;
	};
}

