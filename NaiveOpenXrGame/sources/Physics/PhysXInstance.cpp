#include "PhysXInstance.h"

void Noxg::PhysXInstance::Initialize()
{
	LOG_STEP("PhysX", "Initializing PhysX");
	foundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallback);
	physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), true);
	dispatcher = PxDefaultCpuDispatcherCreate(2);

	PxSceneDesc sceneDesc(physics->getTolerancesScale());
	{
		sceneDesc.gravity = PxVec3(0.0f, 0.0f, 1.0f);
		sceneDesc.cpuDispatcher = dispatcher;
		sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	}
	scene = physics->createScene(sceneDesc);

	material = physics->createMaterial(0.5f, 0.5f, 0.6f);

	if (globalInstance == nullptr) globalInstance = this;
	LOG_SUCCESS();
}

void Noxg::PhysXInstance::Simulate(float timeDelta)
{
	scene->simulate(timeDelta);
	scene->fetchResults(true);
}

PxRigidDynamic* Noxg::PhysXInstance::createRigidDynamic(const PxTransform& pose) const
{
	return physics->createRigidDynamic(pose);
}

void Noxg::PhysXInstance::addActor(PxActor& actor)
{
	scene->addActor(actor);
}
