#include "RigidStatic.h"
#include "Bricks/GameObject.h"
#include "Bricks/SceneManager.h"
#include "PhysicsTransform.h"

void Noxg::RigidStatic::addShape(PxShape* shape)
{
	if (pxRaw == nullptr)
	{
		pending.push_back(shape);
	}
	else
	{
		pxRaw->attachShape(*shape);
	}
}

void Noxg::RigidStatic::Enable()
{
	auto obj = gameObject.lock();
	auto physicsEngineInstance = obj->scene.lock()->manager.lock()->defaultPhysicsEngineInstance;

	glm::mat4 mat = obj->transform->getGlobalMatrix();
	PxTransform pxTransform{ *((PxMat44*)(&mat)) };
	pxRaw = physicsEngineInstance.lock()->createRigidStatic(pxTransform);
	pxRaw->userData = new std::weak_ptr<RigidActor>(std::static_pointer_cast<RigidActor>(shared_from_this()));	// weird though.
	std::dynamic_pointer_cast<PhysicsTransform>(obj->transform)->pxActor = pxRaw;
	for (auto& shape : pending)
	{
		pxRaw->attachShape(*shape);
	}
	obj->scene.lock()->physicsScene->addActor(*pxRaw);
}
