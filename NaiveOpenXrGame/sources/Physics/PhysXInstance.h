#pragma once

#include "mainCommon.h"
#include "PhysicsEngineInstance.h"

namespace Noxg
{
	MAKE_HANDLE(PhysXInstance);

	class PhysXInstance : public PhysicsEngineInstance
	{
	public:
		virtual void Initialize() override;
		virtual void Simulate(float timeDelta) override;
		virtual void addScene(rf::Scene scene) override;
		virtual PxScene* createScene() const override;
		virtual PxRigidDynamic* createRigidDynamic(const PxTransform& pose) const override;
	private:
		PxDefaultAllocator allocator;
		PxDefaultErrorCallback errorCallback;
		PxFoundation* foundation = nullptr;
		PxPhysics* physics = nullptr;
		PxDefaultCpuDispatcher* dispatcher = nullptr;
		PxMaterial* material = nullptr;
	};
}