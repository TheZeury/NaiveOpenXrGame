#include "GameObject.h"

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
