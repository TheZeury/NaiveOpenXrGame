#pragma once

#include "Renderer/GraphicsInstance.h"
#include "Physics/PhysicsEngineInstance.h"
#include "XR/XrInstance.h"
#include "Scene.h"

namespace Noxg
{
	MAKE_HANDLE(SceneManager);

	class SceneManager : public std::enable_shared_from_this<SceneManager>
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
		rf::XrInstance defaultXrInstance;
	};
}