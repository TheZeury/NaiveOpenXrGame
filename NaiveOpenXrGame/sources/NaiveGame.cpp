#include "NaiveGame.h"
#include "XR/XrSpaceTransform.h"
#include "Physics/RigidStatic.h"
#include <chrono>

Noxg::NaiveGame::NaiveGame()
{
}

void Noxg::NaiveGame::Initialize()
{
	graphicsInstance = std::make_shared<VulkanInstance>();
	xrInstance = std::make_shared<OpenXrInstance>(graphicsInstance);
	physicsEngineInstance = std::make_shared<PhysXInstance>();
	sceneManager = std::make_shared<SceneManager>();
	sceneManager->defaultGraphicsInstance = graphicsInstance;
	sceneManager->defaultPhysicsEngineInstance = physicsEngineInstance;

	xrInstance->Initialize();
	graphicsInstance->Initialize(xrInstance);
	physicsEngineInstance->Initialize();
}

void Noxg::NaiveGame::Run()
{
	xrInstance->InitializeSession();
	graphicsInstance->InitializeSession();

	unsigned char blackPixel[4] = { 0, 0, 0, 255 };
	pureBlack = std::make_shared<Texture>(blackPixel, 1, 1, 4);
	unsigned char whitePixel[4] = { 255, 255, 255, 255 };
	pureWhite = std::make_shared<Texture>(whitePixel, 1, 1, 4);

	graphicsInstance->addTexture(pureBlack);
	graphicsInstance->addTexture(pureWhite);
	
	std::vector<Vertex> vertices = {
		// down
		Vertex{ { -0.5f, -0.5f, 0.5f }, { }, { 0.f, -1.f, 0.f }, { } },
		Vertex{ { -0.5f, -0.5f, -0.5f }, { }, { 0.f, -1.f, 0.f }, { } },
		Vertex{ { 0.5f, -0.5f, -0.5f }, { }, { 0.f, -1.f, 0.f }, { } },
		Vertex{ { 0.5f, -0.5f, 0.5f }, { }, { 0.f, -1.f, 0.f }, { } },
		// up
		Vertex{ { -0.5f, 0.5f, 0.5f }, { }, { 0.f, 1.f, 0.f }, { } },
		Vertex{ { 0.5f, 0.5f, 0.5f }, { }, { 0.f, 1.f, 0.f }, { } },
		Vertex{ { 0.5f, 0.5f, -0.5f }, { }, { 0.f, 1.f, 0.f }, { } },
		Vertex{ { -0.5f, 0.5f, -0.5f }, { }, { 0.f, 1.f, 0.f }, { } },
		// front
		Vertex{ { -0.5f, -0.5f, 0.5f }, { }, { 0.f, 0.f, 1.f }, { } },
		Vertex{ { 0.5f, -0.5f, 0.5f }, { }, { 0.f, 0.f, 1.f }, { } },
		Vertex{ { 0.5f, 0.5f, 0.5f }, { }, { 0.f, 0.f, 1.f }, { } },
		Vertex{ { -0.5f, 0.5f, 0.5f }, { }, { 0.f, 0.f, 1.f }, { } },
		// back
		Vertex{ { -0.5f, -0.5f, -0.5f }, { }, { 0.f, 0.f, -1.f }, { } },
		Vertex{ { -0.5f, 0.5f, -0.5f }, { }, { 0.f, 0.f, -1.f }, { } },
		Vertex{ { 0.5f, 0.5f, -0.5f }, { }, { 0.f, 0.f, -1.f }, { } },
		Vertex{ { 0.5f, -0.5f, -0.5f }, { }, { 0.f, 0.f, -1.f }, { } },
		// left
		Vertex{ { -0.5f, -0.5f, 0.5f }, { }, { -1.f, 0.f, 0.f }, { } },
		Vertex{ { -0.5f, 0.5f, 0.5f }, { }, { -1.f, 0.f, 0.f }, { } },
		Vertex{ { -0.5f, 0.5f, -0.5f }, { }, { -1.f, 0.f, 0.f }, { } },
		Vertex{ { -0.5f, -0.5f, -0.5f }, { }, { -1.f, 0.f, 0.f }, { } },
		// right
		Vertex{ { 0.5f, -0.5f, 0.5f }, { }, { 1.f, 0.f, 0.f }, { } },
		Vertex{ { 0.5f, -0.5f, -0.5f }, { }, { 1.f, 0.f, 0.f }, { } },
		Vertex{ { 0.5f, 0.5f, -0.5f }, { }, { 1.f, 0.f, 0.f }, { } },
		Vertex{ { 0.5f, 0.5f, 0.5f }, { }, { 1.f, 0.f, 0.f }, { } },
	};
	std::vector<uint32_t> indices = {
		0 + 0,  1 + 0,  2 + 0,	// down
		2 + 0,  3 + 0,  0 + 0,
		0 + 4,  1 + 4,  2 + 4,	// up
		2 + 4,  3 + 4,  0 + 4,
		0 + 8,  1 + 8,  2 + 8,	// front
		2 + 8,  3 + 8,  0 + 8,
		0 + 12, 1 + 12, 2 + 12,	// back
		2 + 12, 3 + 12, 0 + 12,
		0 + 16, 1 + 16, 2 + 16,	// left
		2 + 16, 3 + 16, 0 + 16,
		0 + 20, 1 + 20, 2 + 20,	// right
		2 + 20, 3 + 20, 0 + 20,
	};
	blackCube = std::make_shared<MeshModel>(vertices, indices, pureBlack);
	whiteCube = std::make_shared<MeshModel>(vertices, indices, pureWhite);
	graphicsInstance->addModel(blackCube);

	auto bulletShape = physicsEngineInstance->createShape(PxBoxGeometry(0.05f, 0.05f, 0.05f));
	auto targetShape = physicsEngineInstance->createShape(PxBoxGeometry(0.5f, 0.5f, 0.5f));

	BuildScene();

	LOG_INFO("GAME", "Entering into Game Loop.", 0);

	auto startTime = std::chrono::high_resolution_clock::now();
	auto lastTime = startTime;

	while (xrInstance->PollEvents())
	{
		if(xrInstance->running())
		{
			auto currentTime = std::chrono::high_resolution_clock::now();
			float timeDelta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
			lastTime = currentTime;

			physicsEngineInstance->Simulate(timeDelta);
			xrInstance->PollActions();
			
			scene->CalculateFrame();

			xrInstance->Update();
			if (Utils::triggerStates[1].changedSinceLastSync == VK_TRUE)
			{
				float value = Utils::triggerStates[1].currentState;
				if (value > 0.8f && Utils::released[1])
				{
					Utils::released[1] = false;
					xr::HapticVibration vibration(xr::Duration::minHaptic(), XR_FREQUENCY_UNSPECIFIED, 1.f);
					xrInstance->vibrate(vibration, 1);

					hd::GameObject bullet = std::make_shared<GameObject>();
					auto pos = *((glm::vec3*)(&(Utils::handLocations[1].pose.position)));
					auto orient = *((glm::quat*)(&(Utils::handLocations[1].pose.orientation)));
					pos = orient * glm::vec3(0.01f, -0.145f, -0.117f) + pos;
					bullet->transform = std::make_shared<PhysicsTransform>(nullptr);
					bullet->transform->setLocalPosition(pos);
					bullet->transform->setLocalRotation(orient);
					auto direction = orient * glm::vec3{ 0.0f, -20.f, 0.0f };
					hd::RigidDynamic rigid = std::make_shared<RigidDynamic>(direction);
					rigid->addShape(bulletShape);
					bullet->addComponent(rigid);

					hd::GameObject bulletModel = std::make_shared<GameObject>();
					bulletModel->models.push_back(blackCube);
					bulletModel->transform->setLocalScale({ 0.1f, 0.1f, 0.1f });
					bullet->transform->addChild(bulletModel->transform);

					scene->addGameObject(bullet);
					scene->addGameObject(bulletModel);
				}
				else if (value < 0.2f && !Utils::released[1])
				{
					Utils::released[1] = true;
				}
			}
			if (Utils::triggerStates[0].changedSinceLastSync == VK_TRUE)
			{
				float value = Utils::triggerStates[0].currentState;
				if (value > 0.8f && Utils::released[0])
				{
					Utils::released[0] = false;
					xr::HapticVibration vibration(xr::Duration::minHaptic(), XR_FREQUENCY_UNSPECIFIED, 1.f);
					xrInstance->vibrate(vibration, 0);

					auto pos = *((glm::vec3*)(&(Utils::handLocations[0].pose.position)));
					auto orient = *((glm::quat*)(&(Utils::handLocations[0].pose.orientation)));
					pos = orient * glm::vec3(0.f, -5.f, 0.f) + pos;

					hd::GameObject box = std::make_shared<GameObject>();
					box->transform = std::make_shared<PhysicsTransform>(nullptr);
					box->transform->setLocalPosition(pos);
					box->transform->setLocalRotation(orient);
					hd::RigidDynamic rigid = std::make_shared<RigidDynamic>();
					rigid->addShape(targetShape);
					box->addComponent(rigid);

					hd::GameObject boxModel = std::make_shared<GameObject>();
					boxModel->models.push_back(whiteCube);
					box->transform->addChild(boxModel->transform);

					scene->addGameObject(box);
					scene->addGameObject(boxModel);
				}
				else if (value < 0.2f && !Utils::released[0])
				{
					Utils::released[0] = true;
				}
			}
		}
	}

	LOG_INFO("GAME", "Exited from Game Loop.", 0);

	blackCube = nullptr;
	whiteCube = nullptr;
	pureBlack = nullptr;
	pureWhite = nullptr;
	scene = nullptr;

	graphicsInstance->CleanUpSession();
	xrInstance->CleanUpSession();
	xrInstance->CleanUpInstance();
	graphicsInstance->CleanUpInstance();
}

void Noxg::NaiveGame::BuildScene()
{
	scene = std::make_shared<Scene>();
	sceneManager->Initialize(scene);

	{	// Ground.
		auto tiles = std::make_shared<Texture>("textures/GroundTile.jpg");
		graphicsInstance->addTexture(tiles);
		std::vector<Vertex> vertices = {
			{ { -100.f, 0.f, 100.f }, { }, { 0.f, 1.f, 0.f }, { 100.f, 0.f } },
			{ { 100.f, 0.f, 100.f }, { }, { 0.f, 1.f, 0.f }, { 0.f, 0.f } },
			{ { 100.f, 0.f, -100.f }, { }, { 0.f, 1.f, 0.f }, { 0.f, 100.f } },
			{ { -100.f, 0.f, -100.f }, { }, { 0.f, 1.f, 0.f }, { 100.f, 100.f } },
		};
		std::vector<uint32_t> indices = {
			0, 1, 2,
			2, 3, 0,
		};
		auto groundModel = std::make_shared<MeshModel>(vertices, indices, tiles);
		auto ground = std::make_shared<GameObject>();
		ground->models.push_back(groundModel);

		auto collider = std::make_shared<GameObject>();
		collider->transform = std::make_shared<PhysicsTransform>(nullptr);
		collider->transform->setLocalRotation(glm::rotate(glm::quat{ 1.f, 0.f, 0.f, 0.f }, glm::pi<float>() / 2.f, { 0.f, 0.f, 1.f }));
		auto rigid = std::make_shared<RigidStatic>();
		auto shape = physicsEngineInstance->createShape(PxPlaneGeometry());
		rigid->addShape(shape);
		collider->addComponent(rigid);

		scene->addGameObject(ground);
		scene->addGameObject(collider);
	}

	{	// Right hand.
		hd::GameObject rightHand = std::make_shared<GameObject>();
		rightHand->transform = std::make_shared<XrSpaceTransform>(Utils::handLocations[1]);

		hd::GameObject revolver = graphicsInstance->loadGameObjectFromFiles("revolver");
		glm::quat rotaA = { 0.7071068f, 0.f, -0.7071068f, 0.f };
		glm::quat rotaB = { 0.7071068f, -0.7071068f, 0.f, 0.f };
		revolver->transform->setLocalRotation(rotaB * rotaA);
		revolver->transform->setLocalPosition({ 0.f, -0.18f, 0.03f });
		revolver->transform->setLocalScale({ 2.54f, 2.54f, 2.54f });	// 0.01 inch to m
		rightHand->transform->addChild(revolver->transform);

		scene->addGameObject(rightHand);
		scene->addGameObject(revolver);
	}

	{	// Steed.
		hd::GameObject steed = std::make_shared<GameObject>();
		steed->transform = std::make_shared<PhysicsTransform>(nullptr);
		steed->transform->setLocalPosition({ -1.f, 0.f, -.5f });
		hd::RigidDynamic rigid = std::make_shared<RigidDynamic>();
		steed->addComponent(rigid);

		hd::GameObject steedModel = graphicsInstance->loadGameObjectFromFiles("steed");
		steedModel->transform->setLocalScale({ .01f, 0.01f, 0.01f });	// cm to m
		steed->transform->addChild(steedModel->transform);

		scene->addGameObject(steed);
		scene->addGameObject(steedModel);
	}

	{
		hd::GameObject box = std::make_shared<GameObject>();
		box->transform = std::make_shared<PhysicsTransform>(nullptr);
		box->transform->setLocalPosition({ 0.f, 1.f, -5.f });
		hd::RigidDynamic rigid = std::make_shared<RigidDynamic>();
		auto shape = physicsEngineInstance->createShape(PxBoxGeometry(0.5f, 0.5f, 0.5f));
		rigid->addShape(shape);
		box->addComponent(rigid);

		hd::GameObject boxModel = std::make_shared<GameObject>();
		boxModel->models.push_back(whiteCube);
		box->transform->addChild(boxModel->transform);

		scene->addGameObject(box);
		scene->addGameObject(boxModel);
	}

	sceneManager->Load(scene);
	sceneManager->Mobilize(scene);
}
