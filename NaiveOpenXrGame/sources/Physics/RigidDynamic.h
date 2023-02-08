#pragma once

#include "mainCommon.h"
#include "Physics/PhysicsEngineInstance.h"
#include "PhysicsTransform.h"
#include "Bricks/GameObject.h"
#include "RigidActor.h"

namespace Noxg
{
	MAKE_HANDLE(RigidDynamic);

	class RigidDynamic : public RigidActor
	{
	public:
		RigidDynamic(glm::vec3 vec = { }, PxForceMode::Enum forceMode = PxForceMode::eVELOCITY_CHANGE);
		~RigidDynamic();
		virtual void addShape(PxShape* shape) override;

	public:
		virtual void Enable() override;
		
	public:
		rf::PhysicsEngineInstance physicsEngineInstance;
		glm::vec3 force;
		PxForceMode::Enum mode;
	private:
		std::vector<PxShape*> pending;
		PxRigidDynamic* pxRaw = nullptr;
	};
}

