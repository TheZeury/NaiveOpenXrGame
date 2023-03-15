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
		void mainLoop(std::stop_token st);
		void fixedLoop(std::stop_token st);
		void BuildScene();
	private:
		hd::Scene scene;
		hd::Material pureBlack;
		hd::Material pureWhite;
		hd::MeshModel blackCube;
		hd::MeshModel whiteCube;
		hd::MeshModel blackSphere;
		hd::MeshModel whiteSphere;
		PxShape* bulletShape;
		rf::GameObject xrOriginObject;
		rf::GameObject cameraObject;

		rf::GameObject revolverObject;
	};
}

