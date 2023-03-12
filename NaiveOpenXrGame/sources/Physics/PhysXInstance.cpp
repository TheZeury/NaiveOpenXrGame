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

PxFilterFlags SimulationFilterShader(
	PxFilterObjectAttributes attributes0, PxFilterData filterData0, 
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	// Let triggers through.
	if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlag::eDEFAULT;
	}

	// Filter test.
	// word0: mask.
	// word1: bits to filter out if counter's mask contains any.
	// word2: bits that counter's mask must contains all.
	// word3: friend mask, ignore word0, word1 and word2 as long as two friend masks overlap.
	if (!(filterData0.word3 & filterData0.word3) && (
		(filterData0.word1 & filterData1.word0) || (filterData1.word1 & filterData0.word0) ||
		(filterData0.word2 & filterData1.word0 ^ filterData0.word2) || (filterData1.word2 & filterData0.word0 ^ filterData1.word2)))
	{
		return PxFilterFlag::eKILL;
	}

	pairFlags = PxPairFlag::eCONTACT_DEFAULT;
	return PxFilterFlag::eDEFAULT;
}

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
		sceneDesc.filterShader = SimulationFilterShader;// PxDefaultSimulationFilterShader;
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

PxShape* Noxg::PhysXInstance::createShape(const PxGeometry& geometry, const PxFilterData& simulationFilterData) const
{
	auto shape = physics->createShape(geometry, *material);
	shape->setSimulationFilterData(simulationFilterData);
	return shape;
}
