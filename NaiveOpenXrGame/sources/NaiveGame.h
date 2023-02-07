#pragma once

#include "mainCommon.h"
#include "Bricks/GameInstance.h"
#include "Renderer/VulkanInstance.h"
#include "XR/OpenXrInstance.h"
#include "Physics/PhysXInstance.h"
#include "Physics/RigidDynamic.h"

namespace Noxg
{
	MAKE_HANDLE(NaiveGame);

	class NaiveGame : public GameInstance
	{
	public:
		NaiveGame();
		virtual void Initialize() override;
		virtual void Run() override;
		void BuildScene();
	private:
		hd::Scene scene;
		hd::Texture pureBlack;
		hd::Texture pureWhite;
		hd::MeshModel blackCube;
		hd::MeshModel whiteCube;
	};
}

