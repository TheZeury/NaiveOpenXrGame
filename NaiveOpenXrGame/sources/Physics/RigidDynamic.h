#pragma once

#include "mainCommon.h"
#include "Physics/PhysicsEngineInstance.h"
#include "PhysicsTransform.h"
#include "Bricks/GameObject.h"

namespace Noxg
{
	MAKE_HANDLE(RigidDynamic);

	class RigidDynamic : public GameComponent
	{
	public:
		RigidDynamic(glm::vec3 vec = { }, PxForceMode::Enum forceMode = PxForceMode::eVELOCITY_CHANGE);
		~RigidDynamic();
		void addShape(PxShape* shape);

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

