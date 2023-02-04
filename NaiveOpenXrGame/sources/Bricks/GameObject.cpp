#include "GameObject.h"

Noxg::GameObject::GameObject()
{
	transform = std::make_shared<Transform>();
}