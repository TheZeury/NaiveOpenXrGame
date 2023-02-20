#include "PhysXInstance.h"
#include "ITriggerCallback.h"

class EventCallback : public PxSimulationEventCallback
{
	virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) { }
	virtual void onWake(PxActor** actors, PxU32 count) { }
	virtual void onSleep(PxActor** actors, PxU32 count) { }
	virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) { }
	virtual void onTrigger(PxTriggerPair* pairs, PxU32 count)
	{
		for (size_t i = 0; i < count; ++i)
		{
			const auto& pair = pairs[i];
			if (pair.triggerShape->userData != nullptr)
			{
				Noxg::ITriggerCallback* triggerCallback = reinterpret_cast<Noxg::ITriggerCallback*>(pair.triggerShape->userData);
				if (pair.status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
				{
					triggerCallback->OnEnter(pair);
				}
				else if (pair.status & PxPairFlag::eNOTIFY_TOUCH_LOST)
				{
					triggerCallback->OnExit(pair);
				}
				else
				{
					LOG_INFO("PhysX", "Undefined.", 0);
				}
			}
		}
	}
	virtual void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) { }
};

void Noxg::PhysXInstance::Initialize()
{
	LOG_STEP("PhysX", "Initializing PhysX");
	foundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallback);
	physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), true);
	dispatcher = PxDefaultCpuDispatcherCreate(PxThread::getNbPhysicalCores());

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
		sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
		sceneDesc.cpuDispatcher = dispatcher;
		sceneDesc.simulationEventCallback = new EventCallback();
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
