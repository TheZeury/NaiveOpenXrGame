#include "RigidDynamic.h"

Noxg::RigidDynamic_T::RigidDynamic_T(GameObject obj) : gameObject{ obj }
{
	glm::mat4 mat = obj->transform->getMatrix();
	PxTransform pxTransform{ *((PxMat44*)(&mat)) };
	pxRaw = PhysXInstance::globalInstance->createRigidDynamic(pxTransform);
	glm::vec3 scale = obj->transform->getScale();
	obj->transform = std::make_shared<PhysicsTransform>(pxRaw);
	obj->transform->setScale(scale);
	PhysXInstance::globalInstance->addActor(*pxRaw);
}

Noxg::RigidDynamic_T::RigidDynamic_T(GameObject obj, glm::vec3 pos, glm::quat rotate) : gameObject{ obj }
{
	PxTransform pxTransform{ *((PxVec3*)(&pos)), *((PxQuat*)(&rotate)) };
	pxRaw = PhysXInstance::globalInstance->createRigidDynamic(pxTransform);
	glm::vec3 scale = obj->transform->getScale();
	obj->transform = std::make_shared<PhysicsTransform>(pxRaw);
	obj->transform->setScale(scale);
	PhysXInstance::globalInstance->addActor(*pxRaw);
}
