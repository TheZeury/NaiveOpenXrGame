#pragma once

#include "Renderer/GraphicsInstance.h"
#include "Physics/PhysicsEngineInstance.h"
#include "Scene.h"

namespace Noxg
{
	MAKE_HANDLE(SceneManager);

	class SceneManager
	{
	public:
		void Initialize(rf::Scene scene);
		void Destroy(rf::Scene scene);
		void Load(rf::Scene scene);
		void Unload(rf::Scene scene);
		void Mobilize(rf::Scene scene);
		void Freeze(rf::Scene scene);

		rf::GraphicsInstance defaultGraphicsInstance;
		rf::PhysicsEngineInstance defaultPhysicsEngineInstance;
	};
}