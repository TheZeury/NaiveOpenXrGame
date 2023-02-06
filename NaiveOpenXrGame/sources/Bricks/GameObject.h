#pragma once

#include "mainCommon.h"
#include "GameComponent.h"
#include "Transform.h"
#include "Renderer/MeshModel.h"

namespace Noxg
{
	class Scene;

	MAKE_HANDLE(GameObject);

	class GameObject : public std::enable_shared_from_this<GameObject>
	{
	public:
		GameObject();

		void addComponent(hd::GameComponent component);

		std::weak_ptr<Scene> scene;

		hd::ITransform transform;

		std::vector<hd::MeshModel> models;

		std::vector<hd::GameComponent> components;
	private:
	};
}

