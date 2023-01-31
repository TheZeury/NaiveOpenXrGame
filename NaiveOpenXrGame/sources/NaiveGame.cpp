#include "NaiveGame.h"
#include <chrono>

Noxg::NaiveGame::NaiveGame() : openXrInstance(vulkanInstance)
{
}

void Noxg::NaiveGame::init()
{
	openXrInstance.Initialize();
	vulkanInstance.Initialize(openXrInstance.getInstance(), openXrInstance.getSystemId());
	physXInstance.Initialize();
}

void Noxg::NaiveGame::run()
{
	openXrInstance.InitializeSession();
	vulkanInstance.InitializeSession();

	LOG_INFO("GAME", "Entering into Game Loop.", 0);

	auto startTime = std::chrono::high_resolution_clock::now();
	auto lastTime = startTime;

	while (openXrInstance.PollEvents())
	{
		if(openXrInstance.running())
		{
			auto currentTime = std::chrono::high_resolution_clock::now();
			float timeDelta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
			lastTime = currentTime;

			physXInstance.Simulate(timeDelta);
			openXrInstance.PoolActions();
			openXrInstance.Update();
		}
	}

	LOG_INFO("GAME", "Exited from Game Loop.", 0);

	vulkanInstance.CleanUpSession();
	openXrInstance.CleanUpSession();
	openXrInstance.CleanUpInstance();
	//vulkanInstance.CleanUpInstance();
}
