#pragma once

#include "mainCommon.h"
#include "Bricks/GameComponent.h"

namespace Noxg
{
	MAKE_HANDLE(RigidActor);

	class RigidActor : public GameComponent
	{
	public:
		virtual void addShape(PxShape* shape) = 0;
	};
}

