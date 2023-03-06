#pragma once

#include "mainCommon.h"
#include "Bricks/GameComponent.h"

namespace Noxg
{
	class XrGrabable : public GameComponent
	{
	public:
		void virtual Enable() override;
	};
}

