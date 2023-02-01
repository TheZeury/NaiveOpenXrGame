#include "GameObject.h"

Noxg::GameObject_T::GameObject_T()
{
	transform = std::make_shared<Transform>();
}

std::shared_ptr<Noxg::ITransform> Noxg::GameObject_T::getTransform()
{
	return transform;
}
