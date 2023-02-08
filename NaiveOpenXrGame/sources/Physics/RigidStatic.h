#pragma once

#include "mainCommon.h"
#include "Physics/RigidActor.h"

namespace Noxg
{
	MAKE_HANDLE(RigidStatic);

	class RigidStatic : public RigidActor
	{
	public:
		virtual void addShape(PxShape* shape) override;

	public:
		virtual void Enable() override;

	private:
		std::vector<PxShape*> pending;
		PxRigidStatic* pxRaw = nullptr;
	};
}

