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
	if (!gameObjects.contains(obj))
	{
		gameObjects.insert(obj);
		obj->scene = shared_from_this();
		obj->transform->Enable();

		for (auto& model : obj->models)
		{
			addModel(model, obj->transform);
		}
		for (auto& text : obj->texts)
		{
			addText(text, obj->transform);
		}
		for (auto& element : obj->uiElements)
		{
			addUIElement(element, obj->transform);
		}

		for (auto& component : obj->components)
		{
			component->Enable();
		}
	}
}

void Noxg::Scene::removeGameObject(hd::GameObject obj)
{
	gameObjects.erase(obj); 
	for (auto& model : obj->models)
	{
		models.erase({ model, obj->transform });
	}
	for (auto& text : obj->texts)
	{
		texts.erase({ text, obj->transform });
	}
	for (auto& element : obj->uiElements)
	{
		uiElements.erase({ element, obj->transform });
	}
}

auto Noxg::Scene::addModel(hd::MeshModel model, hd::ITransform transform) -> void
{
	models.insert({ model, transform });
}

auto Noxg::Scene::addText(hd::TextModel text, hd::ITransform transform) -> void
{
	texts.insert({ text, transform });
}

auto Noxg::Scene::addUIElement(hd::UIElement element, hd::ITransform transform) -> void
{
	uiElements.insert({ element, transform });
}

auto Noxg::Scene::removeModel(hd::MeshModel model, hd::ITransform transform) -> void
{
	models.erase({ model, transform });
}

auto Noxg::Scene::removeText(hd::TextModel text, hd::ITransform transform) -> void
{
	texts.erase({ text, transform });
}

auto Noxg::Scene::removeUIElement(hd::UIElement element, hd::ITransform transform) -> void
{
	uiElements.erase({ element, transform });
}