#include "NaiveGame.h"

Noxg::NaiveGame::NaiveGame() : openXrInstance(vulkanInstance)
{
}

void Noxg::NaiveGame::init()
{
	openXrInstance.Initialize();
	vulkanInstance.Initialize(openXrInstance.getInstance(), openXrInstance.getSystemId());
}

void Noxg::NaiveGame::run()
{
	openXrInstance.InitializeSession();
	vulkanInstance.InitializeSession();

	LOG_INFO("GAME", "Entering into Game Loop.", 0);

	while (openXrInstance.PollEvents())
	{
		if(openXrInstance.running())
		{
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
