#pragma once

#include "Bricks/Transform.h"

namespace Noxg
{
	MAKE_HANDLE(PhysicsTransform);

	class PhysicsTransform : public Transform, public IHaveFrameCalculation
	{
	public:
		virtual std::tuple<bool, glm::mat4*> updateMatrix() override;

		virtual void setGlobalMatrix(const glm::mat4& mat) override;

		PhysicsTransform(PxRigidActor* actor) : pxActor{ actor } { }

	public:
		virtual void CalculateFrame() override;

	private:
		void informChangesToActor();

	private:
		PxRigidActor* pxActor = nullptr;
		friend class RigidDynamic;
		friend class RigidStatic;
	};
}
