#include "RigidDynamic.h"

Noxg::RigidDynamic::RigidDynamic(rf::GameObject obj, rf::PhysicsEngineInstance physicsInstance, PxScene* physicsScene, glm::vec3 vec, PxForceMode::Enum forceMode) : gameObject{ obj }, physicsEngineInstance{ physicsInstance } , force{ vec }, mode{ forceMode }
{
	auto _obj = obj.lock();
	glm::mat4 mat = _obj->transform->getMatrix();
	PxTransform pxTransform{ *((PxMat44*)(&mat)) };
	pxRaw = physicsInstance.lock()->createRigidDynamic(pxTransform);
	glm::vec3 scale = _obj->transform->getScale();
	_obj->transform = std::make_shared<PhysicsTransform>(pxRaw);
	_obj->transform->setScale(scale);
	physicsScene->addActor(*pxRaw);
	if (vec != glm::vec3{ })
	{
		pxRaw->addForce(*((PxVec3*)(&vec)), forceMode);
	}
}

Noxg::RigidDynamic::~RigidDynamic()
{
//	PhysXInstance::globalInstance->removeActor(pxRaw);
}
