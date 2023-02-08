#include "PhysXInstance.h"

void Noxg::PhysXInstance::Initialize()
{
	LOG_STEP("PhysX", "Initializing PhysX");
	foundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallback);
	physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), true);
	dispatcher = PxDefaultCpuDispatcherCreate(2);

	material = physics->createMaterial(0.5f, 0.5f, 0.6f);

	LOG_SUCCESS();
}

void Noxg::PhysXInstance::Simulate(float timeDelta)
{
	for (auto it = scenes.begin(); it != scenes.end(); )
	{
		auto scene = it->lock();
		if (scene == nullptr)
		{
			it = scenes.erase(it);
			continue;
		}
		++it;

		scene->physicsScene->simulate(timeDelta);
	}

	for (auto it = scenes.begin(); it != scenes.end(); ++it)
	{
		auto scene = it->lock();
		if (scene == nullptr) continue;
		scene->physicsScene->fetchResults(true);
	}
}

void Noxg::PhysXInstance::addScene(rf::Scene scene)
{
	scenes.push_back(scene);
}

PxScene* Noxg::PhysXInstance::createScene() const
{
	PxSceneDesc sceneDesc(physics->getTolerancesScale());
	{
		sceneDesc.gravity = PxVec3(0.0f, 0.00f, 0.0f);
		sceneDesc.cpuDispatcher = dispatcher;
		sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	}
	return physics->createScene(sceneDesc);
}

PxRigidDynamic* Noxg::PhysXInstance::createRigidDynamic(const PxTransform& pose) const
{
	return physics->createRigidDynamic(pose);
}

PxRigidStatic* Noxg::PhysXInstance::createRigidStatic(const PxTransform& pose) const
{
	return physics->createRigidStatic(pose);
}

PxShape* Noxg::PhysXInstance::createShape(const PxGeometry& geometry) const
{
	return physics->createShape(geometry, *material);
}
