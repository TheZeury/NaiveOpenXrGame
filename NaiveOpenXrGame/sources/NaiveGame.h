#pragma once

#include "mainCommon.h"
#include "VulkanInstance.h"
#include "OpenXrInstance.h"
#include "Physics/PhysXInstance.h"

namespace Noxg
{
	class NaiveGame
	{
	public:
		NaiveGame();
		void init();
		void run();
	private:
		VulkanInstance vulkanInstance;
		OpenXrInstance openXrInstance;
		PhysXInstance physXInstance;
	};
}

