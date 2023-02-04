#pragma once

#include "mainCommon.h"
#include "Transform.h"
#include "Renderer/MeshModel.h"

namespace Noxg
{
	MAKE_HANDLE(GameObject);

	class GameObject
	{
	public:
		GameObject();

		hd::ITransform transform;

		std::vector<hd::MeshModel> models;
	private:
	};
}

