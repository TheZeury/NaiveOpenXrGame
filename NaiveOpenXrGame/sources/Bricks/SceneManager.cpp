#include "SceneManager.h"

void Noxg::SceneManager::Initialize(rf::Scene scene)
{
	scene.lock()->physicsScene = defaultPhysicsEngineInstance.lock()->createScene();
}

void Noxg::SceneManager::Destroy(rf::Scene scene)
{
	
}

void Noxg::SceneManager::Load(rf::Scene scene)
{
	if (defaultGraphicsInstance.lock() != nullptr)
	{
		defaultGraphicsInstance.lock()->addScene(scene);
	}
}

void Noxg::SceneManager::Unload(rf::Scene scene)
{
	if (defaultGraphicsInstance.lock() != nullptr)
	{
		;
	}
}

void Noxg::SceneManager::Mobilize(rf::Scene scene)
{
	if(defaultPhysicsEngineInstance.lock() != nullptr)
	{
		defaultPhysicsEngineInstance.lock()->addScene(scene);
	}
}

void Noxg::SceneManager::Freeze(rf::Scene scene)
{
}
