#include "Scene.h"
#include "Physics/PhysicsEngineInstance.h"
#include "SceneManager.h"

void Noxg::Scene::CalculateFrame()
{
	for (auto& obj : gameObjects)
	{
		if (auto transform = std::dynamic_pointer_cast<IHaveFrameCalculation>(obj->transform))
		{
			transform->CalculateFrame();
		}

		for (auto& _component : obj->components)
		{
			if (auto component = std::dynamic_pointer_cast<IHaveFrameCalculation>(_component))
			{
				component->CalculateFrame();
			}
		}
	}
}

void Noxg::Scene::addGameObject(hd::GameObject obj)
{
	if (gameObjects.count(obj) == 0)
	{
		gameObjects.insert(obj);
		obj->scene = shared_from_this();
		obj->transform->Enable();
		for (auto& component : obj->components)
		{
			component->Enable();
		}
	}
}

void Noxg::Scene::removeGameObject(hd::GameObject obj)
{
	gameObjects.erase(obj);
}
