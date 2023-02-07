#include "RigidDynamic.h"
#include "Bricks/SceneManager.h"

Noxg::RigidDynamic::RigidDynamic(glm::vec3 vec, PxForceMode::Enum forceMode) : force{ vec }, mode{ forceMode }
{
	
}

Noxg::RigidDynamic::~RigidDynamic()
{
//	PhysXInstance::globalInstance->removeActor(pxRaw);
}

void Noxg::RigidDynamic::addShape(PxShape* shape)
{
	if(pxRaw == nullptr)
	{
		pending.push_back(shape);
	}
	else
	{
		pxRaw->attachShape(*shape);
	}
}

void Noxg::RigidDynamic::Enable()
{
	auto obj = gameObject.lock();
	physicsEngineInstance = obj->scene.lock()->manager.lock()->defaultPhysicsEngineInstance;

	glm::mat4 mat = obj->transform->getGlobalMatrix();
	PxTransform pxTransform{ *((PxMat44*)(&mat)) };
	pxRaw = physicsEngineInstance.lock()->createRigidDynamic(pxTransform);
	std::dynamic_pointer_cast<PhysicsTransform>(obj->transform)->pxActor = pxRaw;
	for (auto& shape : pending)
	{
		pxRaw->attachShape(*shape);
	}
	obj->scene.lock()->physicsScene->addActor(*pxRaw);
	if (force != glm::vec3{ })
	{
		pxRaw->addForce(*((PxVec3*)(&force)), mode);
	}
}
