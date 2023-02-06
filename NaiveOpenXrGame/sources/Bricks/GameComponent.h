#pragma once

#include "mainCommon.h"

namespace Noxg
{
	class GameObject;

	MAKE_HANDLE(GameComponent);

	class GameComponent : public std::enable_shared_from_this<GameComponent>
	{
	public:
		std::weak_ptr<GameObject> gameObject;
		virtual void Enable() = 0;
	};

	class IHaveFrameCalculation
	{
	public:
		virtual void CalculateFrame() = 0;
	};

	class IHavePhysicsCalculation
	{
	public:
		virtual void CalculatePhysics() = 0;
	};
}

