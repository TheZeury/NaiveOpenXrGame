#include "GameObject.h"
#include "Scene.h"

Noxg::GameObject::GameObject()
{
	transform = std::make_shared<Transform>();
}

void Noxg::GameObject::addComponent(hd::GameComponent component)
{
	components.push_back(component);
	component->gameObject = shared_from_this();
	if(scene.lock() != nullptr)
	{
		component->Enable();
	}
}

auto Noxg::GameObject::addModel(hd::MeshModel model) -> void
{
	models.insert(model);
	if (!scene.expired())
	{
		scene.lock()->addModel(model, transform);
	}
}

auto Noxg::GameObject::addText(hd::TextModel text) -> void
{
	texts.insert(text);
	if (!scene.expired())
	{
		scene.lock()->addText(text, transform);
	}
}

auto Noxg::GameObject::addUIElement(hd::UIElement element) -> void
{
	uiElements.insert(element);
	if (!scene.expired())
	{
		scene.lock()->addUIElement(element, transform);
	}
}

auto Noxg::GameObject::removeModel(hd::MeshModel model) -> void
{
	models.erase(model);
	if (!scene.expired())
	{
		scene.lock()->removeModel(model, transform);
	}
}

auto Noxg::GameObject::removeText(hd::TextModel text) -> void
{
	texts.erase(text);
	if (!scene.expired())
	{
		scene.lock()->removeText(text, transform);
	}
}

auto Noxg::GameObject::removeUIElement(hd::UIElement element) -> void
{
	uiElements.erase(element);
	if (!scene.expired())
	{
		scene.lock()->removeUIElement(element, transform);
	}
}