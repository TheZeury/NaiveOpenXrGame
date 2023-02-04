#include "NaiveGame.h"
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

	unsigned char pixel[4] = { 255, 255, 255, 255 };
	auto pureWhite = std::make_shared<Texture>(pixel, 1, 1, 4);
	graphicsInstance->addTexture(pureWhite);
	std::vector<Vertex> vertices = {
		Vertex{ { -0.5f, -0.5f, 0.5f }, { }, { } },
		Vertex{ { 0.5f, -0.5f, 0.5f }, { }, { } },
		Vertex{ { -0.5f, -0.5f, -0.5f }, { }, { } },
		Vertex{ { 0.5f, -0.5f, -0.5f }, { }, { } },
		Vertex{ { -0.5f, 0.5f, 0.5f }, { }, { } },
		Vertex{ { 0.5f, 0.5f, 0.5f }, { }, { } },
		Vertex{ { -0.5f, 0.5f, -0.5f }, { }, { } },
		Vertex{ { 0.5f, 0.5f, -0.5f }, { }, { } },
	};
	std::vector<uint32_t> indices = {
		0, 2, 1,	// down
		1, 2, 3,
		4, 5, 6,	// up
		5, 7, 6,
		0, 1, 4,	// front
		1, 5, 4,
		2, 6, 3,	// back
		3, 6, 7,
		0, 4, 6,	// left
		2, 0, 6,
		1, 3, 7,	// right
		1, 7, 5,
	};
	auto cube = std::make_shared<MeshModel>(vertices, indices, pureWhite);
	graphicsInstance->addModel(cube);

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
					bullet->transform->setPosition({ 0.f, 1.f, 0.f });
					bullet->models.push_back(cube);
					hd::RigidDynamic rigid = std::make_shared<RigidDynamic>(bullet, physicsEngineInstance, scene->physicsScene, glm::vec3{ 0.f, 1.f, -1.f });
					scene->addGameObject(bullet);
				}
				else if (value < 0.2f && !Utils::released[1])
				{
					Utils::released[1] = true;
				}
			}
		}
	}

	LOG_INFO("GAME", "Exited from Game Loop.", 0);

	cube = nullptr;
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

	hd::GameObject revolver = graphicsInstance->loadGameObjectFromFiles("revolver");
	revolver->transform = std::make_shared<XrSpaceTransform>(Utils::handLocations[1]);
	glm::quat rotaA = { 0.7071068f, 0.f, -0.7071068f, 0.f };
	glm::quat rotaB = { 0.7071068f, -0.7071068f, 0.f, 0.f };
	revolver->transform->setRotation(rotaB * rotaA);
	revolver->transform->setPosition({ 0.f, -0.18f, 0.03f });
	revolver->transform->setScale({ 2.54f, 2.54f, 2.54f });	// 0.01 inch to m
	scene->addGameObject(revolver);

	hd::GameObject steed = graphicsInstance->loadGameObjectFromFiles("steed");
	steed->transform->setPosition({ -1.f, 0.f, -.5f });
	steed->transform->setScale({ .01f, .01f, .01f });	// cm to m
	hd::RigidDynamic rigid = std::make_shared<RigidDynamic>(steed, physicsEngineInstance, scene->physicsScene);
	scene->addGameObject(steed);

	sceneManager->Load(scene);
	sceneManager->Mobilize(scene);
}
