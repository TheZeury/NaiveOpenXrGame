#include "Scene.h"
#include "Physics/PhysicsEngineInstance.h"
#include "SceneManager.h"

void Noxg::Scene::addGameObject(hd::GameObject obj)
{
	if (gameObjects.count(obj) == 0)
	{
		gameObjects.insert(obj);
	}
}

void Noxg::Scene::removeGameObject(hd::GameObject obj)
{
	gameObjects.erase(obj);
}
