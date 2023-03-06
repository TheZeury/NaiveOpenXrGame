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

void Noxg::RigidDynamic::addForce(glm::vec3 force, PxForceMode::Enum mode)
{
	if (pxRaw == nullptr) return;
	pxRaw->addForce(*((PxVec3*)(&force)), mode);
}

void Noxg::RigidDynamic::setLinearVelocity(glm::vec3 velocity)
{
	if (pxRaw == nullptr) return;
	pxRaw->setLinearVelocity(*((PxVec3*)(&force)));
}

void Noxg::RigidDynamic::switchGravity(bool enable)
{
	if (pxRaw == nullptr) return;
	pxRaw->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !enable);
}

void Noxg::RigidDynamic::Enable()
{
	auto obj = gameObject.lock();
	physicsEngineInstance = obj->scene.lock()->manager.lock()->defaultPhysicsEngineInstance;

	glm::mat4 mat = obj->transform->getGlobalMatrix();
	PxTransform pxTransform{ *((PxMat44*)(&mat)) };
	pxRaw = physicsEngineInstance.lock()->createRigidDynamic(pxTransform);
	pxRaw->userData = new std::weak_ptr<RigidActor>(std::static_pointer_cast<RigidActor>(shared_from_this()));	// weird.
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
