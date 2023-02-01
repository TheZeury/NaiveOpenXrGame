#pragma once

#include "mainCommon.h"
#include "Renderer/VulkanInstance.h"
#include "XR/OpenXrInstance.h"
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

