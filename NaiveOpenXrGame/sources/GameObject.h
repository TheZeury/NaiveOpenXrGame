#pragma once

#include "mainCommon.h"
#include "Transform.h"
#include "MeshModel.h"

namespace Noxg
{
	class GameObject_T
	{
	public:
		GameObject_T();

		std::shared_ptr<ITransform> getTransform();

		std::shared_ptr<ITransform> transform;

		std::vector<MeshModel> models;
	private:
	};

	using GameObject = std::shared_ptr<GameObject_T>;
}

