#include "NaiveGame.h"
#include "XR/XrSpaceTransform.h"
#include "Physics/RigidStatic.h"
#include "Physics/ITriggerCallback.h"
#include "XR/XrControllerActions.h"
#include "XR/XrGrabber.h"
#include "NaiveGame/MachineGear.h"

Noxg::hd::XrControllerActions rightHandAction;
Noxg::hd::XrControllerActions leftHandAction;

Noxg::hd::GameObject leftHandBox;
Noxg::hd::MachineGear leftHandGear;

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
	sceneManager->defaultXrInstance = xrInstance;

	xrInstance->Initialize();
	graphicsInstance->Initialize(xrInstance);
	physicsEngineInstance->Initialize();
}

void Noxg::NaiveGame::Run()
{
	xrInstance->InitializeSession();
	graphicsInstance->InitializeSession();

	/*unsigned char blackPixel[4] = { 0, 0, 0, 255 };
	pureBlack = std::make_shared<Texture>(blackPixel, 1, 1, 4);
	unsigned char whitePixel[4] = { 255, 255, 255, 255 };
	pureWhite = std::make_shared<Texture>(whitePixel, 1, 1, 4);

	graphicsInstance->addTexture(pureBlack);
	graphicsInstance->addTexture(pureWhite);*/
	pureBlack = std::make_shared<Material>(glm::vec4{ 0.f, 0.f, 0.f, 1.f });
	pureWhite = std::make_shared<Material>(glm::vec4{ 1.f, 1.f, 1.f, 1.f });
	
	std::vector<Vertex> vertices = {
		// down
		Vertex{ { -0.5f, -0.5f, 0.5f }, { }, { }, { 0.f, -1.f, 0.f }, { }, { } },
		Vertex{ { -0.5f, -0.5f, -0.5f }, { }, { }, { 0.f, -1.f, 0.f }, { }, { } },
		Vertex{ { 0.5f, -0.5f, -0.5f }, { }, { }, { 0.f, -1.f, 0.f }, { }, { } },
		Vertex{ { 0.5f, -0.5f, 0.5f }, { }, { }, { 0.f, -1.f, 0.f }, { }, { } },
		// up
		Vertex{ { -0.5f, 0.5f, 0.5f }, { }, { }, { 0.f, 1.f, 0.f }, { } , { }},
		Vertex{ { 0.5f, 0.5f, 0.5f }, { }, { }, { 0.f, 1.f, 0.f }, { } , { }},
		Vertex{ { 0.5f, 0.5f, -0.5f }, { }, { }, { 0.f, 1.f, 0.f }, { } , { }},
		Vertex{ { -0.5f, 0.5f, -0.5f }, { }, { }, { 0.f, 1.f, 0.f }, { } , { }},
		// front
		Vertex{ { -0.5f, -0.5f, 0.5f }, { }, { }, { 0.f, 0.f, 1.f }, { }, { }},
		Vertex{ { 0.5f, -0.5f, 0.5f }, { }, { }, { 0.f, 0.f, 1.f }, { }, { }},
		Vertex{ { 0.5f, 0.5f, 0.5f }, { }, { }, { 0.f, 0.f, 1.f }, { } , { }},
		Vertex{ { -0.5f, 0.5f, 0.5f }, { }, { }, { 0.f, 0.f, 1.f }, { } , { }},
		// back
		Vertex{ { -0.5f, -0.5f, -0.5f }, { }, { }, { 0.f, 0.f, -1.f }, { }, { } },
		Vertex{ { -0.5f, 0.5f, -0.5f }, { }, { }, { 0.f, 0.f, -1.f }, { }, { } },
		Vertex{ { 0.5f, 0.5f, -0.5f }, { }, { }, { 0.f, 0.f, -1.f }, { }, { } },
		Vertex{ { 0.5f, -0.5f, -0.5f }, { }, { }, { 0.f, 0.f, -1.f }, { }, { } },
		// left
		Vertex{ { -0.5f, -0.5f, 0.5f }, { }, { }, { -1.f, 0.f, 0.f }, { }, { } },
		Vertex{ { -0.5f, 0.5f, 0.5f }, { }, { }, { -1.f, 0.f, 0.f }, { }, { } },
		Vertex{ { -0.5f, 0.5f, -0.5f }, { }, { }, { -1.f, 0.f, 0.f }, { }, { } },
		Vertex{ { -0.5f, -0.5f, -0.5f }, { }, { }, { -1.f, 0.f, 0.f }, { }, { } },
		// right
		Vertex{ { 0.5f, -0.5f, 0.5f }, { }, { }, { 1.f, 0.f, 0.f }, { }, { }},
		Vertex{ { 0.5f, -0.5f, -0.5f }, { }, { }, { 1.f, 0.f, 0.f }, { }, { }},
		Vertex{ { 0.5f, 0.5f, -0.5f }, { }, { }, { 1.f, 0.f, 0.f }, { }, { }},
		Vertex{ { 0.5f, 0.5f, 0.5f }, { }, { }, { 1.f, 0.f, 0.f }, { }, { }},
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
	/*graphicsInstance->addModel(blackCube);*/

	auto bulletShape = physicsEngineInstance->createShape(PxBoxGeometry(0.05f, 0.05f, 0.05f));

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

			physicsEngineInstance->Simulate(0.02f);
			xrInstance->PollActions();
			
			scene->CalculateFrame();

			{
				leftHandBox->transform->setLocalPosition({ 0.f, -5.f * (leftHandAction->gripValue()), 0.f });

				glm::vec2 angularVelocity = leftHandAction->primaryAxisValue();
				if(angularVelocity != glm::vec2{ })
				{
					glm::quat rotation = glm::rotate(glm::quat{ 1.f, 0.f, 0.f, 0.f }, glm::length(angularVelocity) * timeDelta, glm::normalize(glm::vec3{-angularVelocity.y, 0.f, -angularVelocity.x}));
					leftHandBox->transform->setLocalRotation(rotation * leftHandBox->transform->getLocalRotation());
				}
			}

			if (leftHandAction->primaryButtonClicked())
			{
				leftHandGear->setLevel(leftHandGear->level - 1);
			}

			if (leftHandAction->secondaryButtonClicked())
			{
				leftHandGear->setLevel(leftHandGear->level + 1);
			}

			if (rightHandAction->triggerClicked())
			{
				xr::HapticVibration vibration(xr::Duration::minHaptic(), XR_FREQUENCY_UNSPECIFIED, 1.f);
				xrInstance->vibrate(vibration, 1);

				hd::GameObject bullet = std::make_shared<GameObject>();
				auto pos = rightHandAction->position();
				auto orient = rightHandAction->rotation();
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
			if (leftHandAction->triggerClicked())
			{
				xr::HapticVibration vibration(xr::Duration::minHaptic(), XR_FREQUENCY_UNSPECIFIED, 1.f);
				xrInstance->vibrate(vibration, 0);

				hd::GameObject box = std::make_shared<GameObject>();
				box->transform = std::make_shared<PhysicsTransform>(nullptr);
				box->transform->setLocalMatrix(leftHandBox->transform->getGlobalMatrix());
				box->transform->setLocalScale({ 1.f, 1.f, 1.f });
				hd::RigidDynamic rigid = std::make_shared<RigidDynamic>();
				auto shapes = leftHandGear->getRecommendedColliders();
				for(auto& shape : shapes)
				{
					rigid->addShape(shape);
				}
				box->addComponent(rigid);

				hd::GameObject boxModel = std::make_shared<GameObject>();
				boxModel->models.push_back(leftHandBox->models[0]);
				boxModel->transform->setLocalScale(leftHandBox->transform->getLocalScale());
				box->transform->addChild(boxModel->transform);

				scene->addGameObject(box);
				scene->addGameObject(boxModel);
			}

			xrInstance->Update();
		}
	}

	LOG_INFO("GAME", "Exited from Game Loop.", 0);

	leftHandBox = nullptr;
	leftHandAction = nullptr;
	rightHandAction = nullptr;
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
		auto tiles_diffuse = std::make_shared<Texture>("textures/GroundTile_diffuse.jpg");
		auto tiles_normal = std::make_shared<Texture>("textures/GroundTile_normal.jpg");
		auto tiles = std::make_shared<Material>(tiles_diffuse, tiles_normal);
		/*graphicsInstance->addTexture(tiles);*/
		std::vector<Vertex> vertices = {
			{ { -100.f, 0.f, 100.f }, { }, { 100.f, 0.f }, { 0.f, 1.f, 0.f }, { }, { } },
			{ { 100.f, 0.f, 100.f }, { }, { 0.f, 0.f }, { 0.f, 1.f, 0.f }, { }, { } },
			{ { 100.f, 0.f, -100.f },{ }, { 0.f, 100.f }, { 0.f, 1.f, 0.f }, { }, { } },
			{ { -100.f, 0.f, -100.f }, { }, { 100.f, 100.f }, { 0.f, 1.f, 0.f }, { }, { } },
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
		auto rigid = std::make_shared<RigidStatic>();
		auto shape = physicsEngineInstance->createShape(PxPlaneGeometry());
		shape->setLocalPose(PxTransform(PxShortestRotation({ 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f })));
		rigid->addShape(shape);
		collider->addComponent(rigid);

		scene->addGameObject(ground);
		scene->addGameObject(collider);
	}

	{	// Right hand.
		hd::GameObject rightHand = std::make_shared<GameObject>();
		rightHandAction = std::make_shared<XrControllerActions>(1);
		rightHand->addComponent(rightHandAction);

		//hd::GameObject revolver = graphicsInstance->loadGameObjectFromFiles("revolver");
		//glm::quat rotaA = { 0.7071068f, 0.f, -0.7071068f, 0.f };
		//glm::quat rotaB = { 0.7071068f, -0.7071068f, 0.f, 0.f };
		//revolver->transform->setLocalRotation(rotaB * rotaA);
		//revolver->transform->setLocalPosition({ 0.f, -0.18f, 0.03f });
		//revolver->transform->setLocalScale({ 2.54f, 2.54f, 2.54f });	// 0.01 inch to m
		//rightHand->transform->addChild(revolver->transform);

		hd::XrGrabber grabber = std::make_shared<XrGrabber>(rightHandAction, PxBoxGeometry(0.1f, 0.1f, 0.1f));
		rightHand->addComponent(grabber);

		hd::GameObject box = std::make_shared<GameObject>();
		box->transform->setLocalScale({ 0.1f, 0.1f, 0.1f });
		box->models.push_back(whiteCube);
		rightHand->transform->addChild(box->transform);

		scene->addGameObject(rightHand);
		//scene->addGameObject(revolver);
		scene->addGameObject(box);
	}

	{	// Left hand.
		hd::GameObject leftHand = std::make_shared<GameObject>();
		leftHandAction = std::make_shared<XrControllerActions>(0);
		leftHand->addComponent(leftHandAction);

		/*hd::GameObject triggerObject = std::make_shared<GameObject>();
		triggerObject->transform = std::make_shared<PhysicsTransform>(nullptr);
		hd::RigidStatic rigid = std::make_shared<RigidStatic>();
		auto shape = physicsEngineInstance->createShape(PxSphereGeometry(0.1f));
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
		class TempTriggerCallback: public ITriggerCallback
		{
			virtual void OnEnter(const PxTriggerPair& pair) override { LOG_INFO("PhysX", "Trigger Entered.", 0); };
			virtual void OnExit(const PxTriggerPair& pair) override { LOG_INFO("PhysX", "Trigger Exited.", 0); };
		};
		shape->userData = static_cast<ITriggerCallback*>(new TempTriggerCallback());
		rigid->addShape(shape);
		triggerObject->addComponent(rigid);
		leftHand->transform->addChild(triggerObject->transform);*/

		leftHandBox = std::make_shared<GameObject>();
		leftHandGear = std::make_shared<MachineGear>(pureWhite);
		leftHandBox->addComponent(leftHandGear);
		leftHand->transform->addChild(leftHandBox->transform);

		scene->addGameObject(leftHand);
		//scene->addGameObject(triggerObject);
		scene->addGameObject(leftHandBox);
	}

	{	// Steed.
		hd::GameObject steed = std::make_shared<GameObject>();
		//steed->transform = std::make_shared<PhysicsTransform>(nullptr);
		steed->transform->setLocalPosition({ -1.f, 0.f, -.5f });
		//hd::RigidDynamic rigid = std::make_shared<RigidDynamic>();
		//steed->addComponent(rigid);

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
