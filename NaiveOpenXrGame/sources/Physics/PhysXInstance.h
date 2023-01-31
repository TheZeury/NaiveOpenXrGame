#pragma once

#include "..\mainCommon.h"

namespace Noxg
{
	class PhysXInstance
	{
	public:
		void Initialize();
		void Simulate(float timeDelta);
		PxRigidDynamic* createRigidDynamic(const PxTransform& pose) const;
		void addActor(PxActor& actor);
		inline static PhysXInstance* globalInstance = nullptr;
	private:
		PxDefaultAllocator allocator;
		PxDefaultErrorCallback errorCallback;
		PxFoundation* foundation = nullptr;
		PxPhysics* physics = nullptr;
		PxDefaultCpuDispatcher* dispatcher = nullptr;
		PxScene* scene = nullptr;
		PxMaterial* material = nullptr;
	};
}