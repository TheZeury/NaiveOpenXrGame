#pragma once

#include "../mainCommon.h"
#include "PhysXInstance.h"
#include "PhysicsTransform.h"
#include "../GameObject.h"

namespace Noxg
{
	class RigidDynamic_T
	{
	public:
		RigidDynamic_T(GameObject obj);
		RigidDynamic_T(GameObject obj, glm::vec3 pos, glm::quat rotate);
		std::weak_ptr<GameObject_T> gameObject;
	private:
		PxRigidDynamic* pxRaw = nullptr;
	};

	using RigidDynamic = std::shared_ptr<RigidDynamic_T>;
}

