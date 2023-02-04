#pragma once

#include "mainCommon.h"
#include "Renderer/GraphicsInstance.h"
#include "XR/XrInstance.h"
#include "SceneManager.h"

namespace Noxg
{
	MAKE_HANDLE(GameInstance);

	class GameInstance
	{
	public:
		virtual void Initialize() = 0;
		virtual void Run() = 0;
	protected:
		hd::XrInstance xrInstance;
		hd::GraphicsInstance graphicsInstance;
		hd::PhysicsEngineInstance physicsEngineInstance;
		hd::SceneManager sceneManager;
	};
}

