#include "NaiveGame.h"
#include "XR/XrSpaceTransform.h"
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

	unsigned char pixel[4] = { 0, 0, 0, 255 };
	auto pureBlack = std::make_shared<Texture>(pixel, 1, 1, 4);
	graphicsInstance->addTexture(pureBlack);
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
	auto cube = std::make_shared<MeshModel>(vertices, indices, pureBlack);
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
					bullet->addComponent(rigid);

					hd::GameObject bulletModel = std::make_shared<GameObject>();
					bulletModel->models.push_back(cube);
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
		}
	}

	LOG_INFO("GAME", "Exited from Game Loop.", 0);

	cube = nullptr;
	pureBlack = nullptr;
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

	hd::GameObject steed = std::make_shared<GameObject>();
	steed->transform = std::make_shared<PhysicsTransform>(nullptr);
	steed->transform->setLocalPosition({ -1.f, 0.f, -.5f });
	hd::RigidDynamic rigid = std::make_shared<RigidDynamic>(glm::vec3{ -5.f, 0.f, -5.f });
	steed->addComponent(rigid);

	hd::GameObject steedModel = graphicsInstance->loadGameObjectFromFiles("steed");
	steedModel->transform->setLocalScale({ .01f, 0.01f, 0.01f });	// cm to m
	steed->transform->addChild(steedModel->transform);
	std::cout << (steedModel->transform->getGlobalMatrix() * glm::vec4(0.f, 0.f, 0.f, 1.f)).x << std::endl;

	scene->addGameObject(steed);
	scene->addGameObject(steedModel);

	sceneManager->Load(scene);
	sceneManager->Mobilize(scene);
}
