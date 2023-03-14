#include "NaiveGame.h"
#include "Physics/RigidStatic.h"
#include "Physics/ITriggerCallback.h"
#include "XR/XrControllerActions.h"
#include "XR/XrGrabber.h"
#include "XR/XrGrabable.h"
#include "NaiveGame/MachineGear.h"
#include "Renderer/MeshModel.h"

Noxg::rf::GameObject rightHandObject;
Noxg::rf::GameObject leftHandObject;

Noxg::rf::GameObject selfControllerObject;
Noxg::rf::RigidDynamic selfControllerRigid;

Noxg::hd::XrControllerActions rightHandAction;
Noxg::hd::XrControllerActions leftHandAction;

//Noxg::hd::GameObject leftHandBox;
//Noxg::hd::MachineGear leftHandGear;

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

	pureBlack = std::make_shared<Material>(glm::vec4{ 0.f, 0.f, 0.f, 1.f });
	pureWhite = std::make_shared<Material>(glm::vec4{ 1.f, 1.f, 1.f, 1.f });
	
	MeshBuilder cubeBuilder = MeshBuilder::Box(0.5f, 0.5f, 0.5f);
	blackCube = cubeBuilder.build(pureBlack);
	whiteCube = cubeBuilder.build(pureWhite);
	MeshBuilder sphereBuilder = MeshBuilder::Icosphere(0.5f, 3);
	blackSphere = sphereBuilder.build(pureBlack);
	whiteSphere = sphereBuilder.build(pureWhite);

	bulletShape = physicsEngineInstance->createShape(PxSphereGeometry(0.05f), NaiveGameSimulationFilters::CommonInWorld);

	BuildScene();

	LOG_INFO("Game", "Entering into Main Loop.", 0);
	std::jthread main{ [&] { mainLoop(); } };
	LOG_INFO("Game", "Entering into Fixed Loop.", 0);
	std::jthread fixed{ [&](std::stop_token st) { fixedLoop(st); } };
	main.join();
	LOG_INFO("Game", "Exited from Main Loop.", 0);
	fixed.request_stop();
	fixed.join();
	LOG_INFO("Game", "Exited from Fixed Loop.", 0);
	
	//leftHandBox = nullptr;
	leftHandAction = nullptr;
	rightHandAction = nullptr;
	blackSphere = nullptr;
	whiteSphere = nullptr;
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

void Noxg::NaiveGame::mainLoop()
{
	auto startTime = std::chrono::high_resolution_clock::now();
	auto lastTime = startTime;

#ifdef MIRROR_WINDOW
	while (xrInstance->PollEvents() && graphicsInstance->PollEvents())
#else
	while (xrInstance->PollEvents())
#endif
	{
		if (xrInstance->running())
		{
			auto currentTime = std::chrono::high_resolution_clock::now();
			float timeDelta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
			lastTime = currentTime;

			physicsEngineInstance->Simulate(0.02f);
			xrInstance->PollActions();

			scene->CalculateFrame();

			/*{
				leftHandBox->transform->setLocalPosition({ 0.f, -5.f * (leftHandAction->gripValue()), 0.f });

				glm::vec2 angularVelocity = leftHandAction->primaryAxisValue();
				if (angularVelocity != glm::vec2{ })
				{
					glm::quat rotation = glm::rotate(glm::quat{ 1.f, 0.f, 0.f, 0.f }, glm::length(angularVelocity) * timeDelta, glm::normalize(glm::vec3{ -angularVelocity.y, 0.f, -angularVelocity.x }));
					leftHandBox->transform->setLocalRotation(rotation * leftHandBox->transform->getLocalRotation());
				}
			}*/

			/*if(!xrOriginObject.expired() && !cameraObject.expired())
			{
				if (rightHandAction->primaryButtonClicked())
				{
					auto matrix = rightHandObject.lock()->transform->getGlobalMatrix();
					glm::quat orient;
					XrMatrix4x4f_GetRotation((XrQuaternionf*)(&orient), (XrMatrix4x4f*)(&matrix));
					xrOriginObject.lock()->transform->setLocalPosition(xrOriginObject.lock()->transform->getLocalPosition() + orient * glm::vec3{  0.f,  1.f,  0.f });
				}

				if (rightHandAction->secondaryButtonClicked())
				{
					auto matrix = rightHandObject.lock()->transform->getGlobalMatrix();
					glm::quat orient;
					XrMatrix4x4f_GetRotation((XrQuaternionf*)(&orient), (XrMatrix4x4f*)(&matrix));
					xrOriginObject.lock()->transform->setLocalPosition(xrOriginObject.lock()->transform->getLocalPosition() + orient * glm::vec3{  0.f, -1.f,  0.f });
				}
			}*/

			/*if (leftHandAction->primaryButtonClicked())
			{
				leftHandGear->setLevel(leftHandGear->level - 1);
			}

			if (leftHandAction->secondaryButtonClicked())
			{
				leftHandGear->setLevel(leftHandGear->level + 1);
			}*/

			/*if (rightHandAction->triggerClicked())
			{
				auto pos = revolverObject.lock()->transform->getLocalPosition();
				auto rotate = revolverObject.lock()->transform->getLocalRotation();
				LOG_INFO("Game", std::format("Pos: [ {}, {}, {} ]", pos.x, pos.y, pos.z), 0);
				LOG_INFO("Game", std::format("Rotate: [ {}, {}, {}, {} ]", rotate.w, rotate.x, rotate.y, rotate.z), 0);
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
				for (auto& shape : shapes)
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
			}*/

			xrInstance->Update();
		}
	}
}

void Noxg::NaiveGame::fixedLoop(std::stop_token st)
{
	namespace chrono = std::chrono;
	using namespace std::literals::chrono_literals;

	const auto fixedTimeDelta = 20ms;
	auto targetTime = chrono::high_resolution_clock::now();

	int i = 0;
	while (!st.stop_requested())
	{
		auto now = chrono::high_resolution_clock::now();
		if (targetTime <= now)
		{
			// TODO. Do someting.
			++i;
			if (i == 500)
			{
				LOG_INFO("Game", "10 seconds of fixed loop.", 0);
				i = 0;
			}

			targetTime = targetTime + fixedTimeDelta;
			if (now - targetTime > 1s) // To avoid rushing advance after pausing or lagging.
			{
				targetTime = now + fixedTimeDelta;
			}
			chrono::high_resolution_clock::duration remainDelta = targetTime - chrono::high_resolution_clock::now();
			std::this_thread::sleep_for(remainDelta);
		}
		else if (targetTime - now > fixedTimeDelta)	// In case that something unexpected happend that made the targetTime far later than now. Normally this can't happen.
		{
			targetTime = now + fixedTimeDelta;
			LOG_ERRO("Unexpected time change. Fixed loop realigned.");
			chrono::high_resolution_clock::duration remainDelta = targetTime - chrono::high_resolution_clock::now();
			std::this_thread::sleep_for(remainDelta);
		}
	}
}

void Noxg::NaiveGame::BuildScene()
{
	scene = std::make_shared<Scene>();
	sceneManager->Initialize(scene);

	{	// XR Origin
		auto origin = std::make_shared<GameObject>();
		auto camera = std::make_shared<GameObject>();
		origin->transform->addChild(camera->transform);
		
		scene->addGameObject(origin);
		scene->addGameObject(camera);

		xrOriginObject = origin;
		cameraObject = camera;
		scene->cameraTransform = camera->transform;
	}

	{	// Ground.
		auto tiles_diffuse = std::make_shared<Texture>("textures/GroundTile_diffuse.jpg");
		auto tiles_normal = std::make_shared<Texture>("textures/GroundTile_normal.jpg");
		auto tiles = std::make_shared<Material>(tiles_diffuse, tiles_normal);
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
		auto shape = physicsEngineInstance->createShape(PxPlaneGeometry(), NaiveGameSimulationFilters::CommonInWorld);
		shape->setLocalPose(PxTransform(PxShortestRotation({ 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f })));
		rigid->addShape(shape);
		collider->addComponent(rigid);

		scene->addGameObject(ground);
		scene->addGameObject(collider);
	}

	{	// Right hand.
		hd::GameObject rightHand = std::make_shared<GameObject>();
		if (!xrOriginObject.expired())
		{
			xrOriginObject.lock()->transform->addChild(rightHand->transform);
		}
		rightHandAction = std::make_shared<XrControllerActions>(1);
		rightHand->addComponent(rightHandAction);

		hd::XrGrabber grabber = std::make_shared<XrGrabber>(rightHandAction, PxBoxGeometry(0.1f, 0.1f, 0.1f));
		rightHand->addComponent(grabber);

		auto whiteCone = MeshBuilder::Cone(0.5f, 0.1f, 1.f, 16).build(pureWhite);

		hd::GameObject rightHandModel = std::make_shared<GameObject>();
		rightHandModel->transform->setLocalScale({ 0.1f, 0.1f, 0.1f });
		rightHandModel->models.push_back(whiteCone);
		rightHand->transform->addChild(rightHandModel->transform);

		rightHandObject = rightHand;
		scene->addGameObject(rightHand);
		scene->addGameObject(rightHandModel);
	}

	{	// Left hand.
		hd::GameObject leftHand = std::make_shared<GameObject>();
		if (!xrOriginObject.expired())
		{
			xrOriginObject.lock()->transform->addChild(leftHand->transform);
		}
		leftHandAction = std::make_shared<XrControllerActions>(0);
		leftHand->addComponent(leftHandAction);

		hd::XrGrabber grabber = std::make_shared<XrGrabber>(leftHandAction, PxBoxGeometry(0.1f, 0.1f, 0.1f));
		leftHand->addComponent(grabber);

		auto blackCone = MeshBuilder::Cone(0.5f, 0.1f, 1.f, 16).build(pureBlack);

		hd::GameObject leftHandModel = std::make_shared<GameObject>();
		leftHandModel->transform->setLocalScale({ 0.1f, 0.1f, 0.1f });
		leftHandModel->models.push_back(blackCone);
		leftHand->transform->addChild(leftHandModel->transform);

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

		/*leftHandBox = std::make_shared<GameObject>();
		leftHandGear = std::make_shared<MachineGear>(pureWhite);
		leftHandBox->addComponent(leftHandGear);
		leftHand->transform->addChild(leftHandBox->transform);*/

		leftHandObject = leftHand;
		scene->addGameObject(leftHand);
		//scene->addGameObject(triggerObject);
		scene->addGameObject(leftHandModel);
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

	{	// Cube
		hd::GameObject box = std::make_shared<GameObject>();
		box->transform = std::make_shared<PhysicsTransform>(nullptr);
		box->transform->setLocalPosition({ -1.f, 1.f, -5.f });
		hd::RigidDynamic rigid = std::make_shared<RigidDynamic>();
		auto shape = physicsEngineInstance->createShape(PxBoxGeometry(0.5f, 0.5f, 0.5f), NaiveGameSimulationFilters::CommonInWorld);
		rigid->addShape(shape);
		box->addComponent(rigid);
		hd::XrGrabable grabable = std::make_shared<XrGrabable>();
		grabable->freeGrabbing = true;
		rigid->grabable = grabable;
		box->addComponent(grabable);

		hd::GameObject boxModel = std::make_shared<GameObject>();
		boxModel->models.push_back(whiteCube);
		box->transform->addChild(boxModel->transform);

		scene->addGameObject(box);
		scene->addGameObject(boxModel);
	}

	{	// Sphere
		hd::GameObject sphere = std::make_shared<GameObject>();
		sphere->transform = std::make_shared<PhysicsTransform>(nullptr);
		sphere->transform->setLocalPosition({ 1.f, 1.f, -5.f });
		hd::RigidDynamic rigid = std::make_shared<RigidDynamic>();
		auto shape = physicsEngineInstance->createShape(PxSphereGeometry(0.5f), NaiveGameSimulationFilters::CommonInWorld);
		rigid->addShape(shape);
		sphere->addComponent(rigid);
		hd::XrGrabable grabable = std::make_shared<XrGrabable>();
		grabable->freeGrabbing = true;
		rigid->grabable = grabable;
		sphere->addComponent(grabable);

		hd::GameObject sphereModel = std::make_shared<GameObject>();
		sphereModel->models.push_back(whiteSphere);
		sphere->transform->addChild(sphereModel->transform);

		scene->addGameObject(sphere);
		scene->addGameObject(sphereModel);
	}

	{	// Cone
		hd::GameObject cone = std::make_shared<GameObject>();
		cone->transform = std::make_shared<PhysicsTransform>(nullptr);
		cone->transform->setLocalPosition({ 3.f, 1.f, -5.f });
		hd::RigidDynamic rigid = std::make_shared<RigidDynamic>();
		auto shape = physicsEngineInstance->createShape(PxSphereGeometry(0.5f), NaiveGameSimulationFilters::CommonInWorld);
		rigid->addShape(shape);
		cone->addComponent(rigid);
		hd::XrGrabable grabable = std::make_shared<XrGrabable>();
		grabable->freeGrabbing = true;
		rigid->grabable = grabable;
		cone->addComponent(grabable);

		auto whiteCone = MeshBuilder::Cone(0.5f, 0.2f, 1.f, 12).build(pureWhite);

		hd::GameObject coneModel = std::make_shared<GameObject>();
		coneModel->models.push_back(whiteCone);
		cone->transform->addChild(coneModel->transform);

		scene->addGameObject(cone);
		scene->addGameObject(coneModel);
	}

	{	// Wooden Table
		hd::GameObject table = std::make_shared<GameObject>();
		table->transform = std::make_shared<PhysicsTransform>(nullptr);
		table->transform->setLocalPosition({ 1.f, 0.f, 0.f });
		auto rigid = std::make_shared<RigidStatic>();
		auto shape = physicsEngineInstance->createShape(PxBoxGeometry(0.56f, 0.49f, 1.2f), NaiveGameSimulationFilters::CommonInWorld);
		shape->setLocalPose(PxTransform(PxVec3{ 0.f, 0.49f, 0.f }));
		rigid->addShape(shape);
		table->addComponent(rigid);

		auto tableModel = graphicsInstance->loadGameObjectFromFiles("wooden_table");
		table->transform->addChild(tableModel->transform);

		scene->addGameObject(table);
		scene->addGameObject(tableModel);
	}

	{	// Revolver
		hd::GameObject revolver = std::make_shared<GameObject>(); 
		revolverObject = revolver;
		revolver->transform = std::make_shared<PhysicsTransform>(nullptr);
		revolver->transform->setLocalPosition({ 0.7025244f, 1.0002166f, -0.0175f });
		revolver->transform->setLocalRotation({ -0.039216336f, -0.038507517f, 0.70616245f, -0.7059135f });
		hd::RigidDynamic rigid = std::make_shared<RigidDynamic>();
		auto shape = physicsEngineInstance->createShape(PxBoxGeometry(0.2f, 0.1f, 0.02f), NaiveGameSimulationFilters::CommonInWorld);
		shape->setLocalPose(PxTransform(PxVec3{ 0.f, 0.05f, 0.f }));
		rigid->addShape(shape);
		revolver->addComponent(rigid);
		hd::XrGrabable grabable = std::make_shared<XrGrabable>();
		grabable->freeGrabbing = false;
		{	// Using a temporary Transform to help calculating the attachTransformation.
			glm::quat rotaA = { 0.7071068f, 0.f, -0.7071068f, 0.f };
			glm::quat rotaB = { 0.7071068f, -0.7071068f, 0.f, 0.f };
			glm::vec3 offset = { 0.f, -0.18f, 0.03f };
			hd::Transform tempTransform = std::make_shared<Transform>();
			tempTransform->setLocalRotation(rotaB * rotaA);
			tempTransform->setLocalPosition(offset);
			grabable->attachTransformation = tempTransform->getLocalMatrix();
		}
		auto RevolverCalculateFrame = [&](hd::XrControllerActions controller)
		{
			if (controller->triggerClicked())
			{
				controller->vibrate();

				hd::GameObject bullet = std::make_shared<GameObject>();
				bullet->transform = std::make_shared<PhysicsTransform>(nullptr);
				glm::vec3 pos{ -0.25f, 0.14f, 0.f };
				bullet->transform->setLocalPosition(revolverObject.lock()->transform->getGlobalMatrix() * glm::vec4(pos, 1.f));
				glm::quat orient = revolverObject.lock()->transform->getLocalRotation();
				auto direction = orient * glm::vec3{ -20.0f, 0.f, 0.0f };
				hd::RigidDynamic rigid = std::make_shared<RigidDynamic>(direction);
				rigid->addShape(bulletShape);
				bullet->addComponent(rigid);

				hd::GameObject bulletModel = std::make_shared<GameObject>();
				bulletModel->models.push_back(blackSphere);
				bulletModel->transform->setLocalScale({ 0.1f, 0.1f, 0.1f });
				bullet->transform->addChild(bulletModel->transform);

				scene->addGameObject(bullet);
				scene->addGameObject(bulletModel);
			}
		};
		grabable->GrabbingFrameCalculateFunction = RevolverCalculateFrame;
		rigid->grabable = grabable;
		revolver->addComponent(grabable);

		hd::GameObject revolverModel = graphicsInstance->loadGameObjectFromFiles("revolver");
		revolverModel->transform->setLocalPosition({ 0.0f, 0.f, 0.01f });
		revolverModel->transform->setLocalScale({ 2.54f, 2.54f, 2.54f });	// 0.01 inch to m
		revolver->transform->addChild(revolverModel->transform);

		scene->addGameObject(revolver);
		scene->addGameObject(revolverModel);
	}

	{	// Self Controller ( Provide movement )
		hd::GameObject selfController = std::make_shared<GameObject>();
		selfControllerObject = selfController;
		selfController->transform = std::make_shared<PhysicsTransform>(nullptr);
		selfController->transform->setLocalPosition({ -0.2f, 1.0f, -0.2f });
		hd::RigidDynamic rigid = std::make_shared<RigidDynamic>();
		selfControllerRigid = rigid;
		auto shape = physicsEngineInstance->createShape(PxSphereGeometry(0.05f), NaiveGameSimulationFilters::CommonUIHovering);
		rigid->addShape(shape);
		selfController->addComponent(rigid);
		hd::XrGrabable grabable = std::make_shared<XrGrabable>();
		grabable->freeGrabbing = false;
		auto selfControllerCalculateFrame = [&](hd::XrControllerActions controller)
		{
			if (!xrOriginObject.expired() && !cameraObject.expired())
			{
				if (controller->primaryButtonClicked())
				{
					auto matrix = controller->gameObject.lock()->transform->getGlobalMatrix();
					glm::quat orient;
					XrMatrix4x4f_GetRotation((XrQuaternionf*)(&orient), (XrMatrix4x4f*)(&matrix));
					xrOriginObject.lock()->transform->setLocalPosition(xrOriginObject.lock()->transform->getLocalPosition() + orient * glm::vec3{ 0.f,  1.f,  0.f });
				}

				if (controller->secondaryButtonClicked())
				{
					auto matrix = controller->gameObject.lock()->transform->getGlobalMatrix();
					glm::quat orient;
					XrMatrix4x4f_GetRotation((XrQuaternionf*)(&orient), (XrMatrix4x4f*)(&matrix));
					xrOriginObject.lock()->transform->setLocalPosition(xrOriginObject.lock()->transform->getLocalPosition() + orient * glm::vec3{ 0.f, -1.f,  0.f });
				}
			}
		};
		auto selfControllerOnRelease = [&](hd::XrControllerActions controller)
		{
			selfControllerRigid.lock()->switchGravity(false);
			selfControllerObject.lock()->transform->setLocalPosition({ -0.2f, 1.0f, -0.2f });
		};
		grabable->GrabbingFrameCalculateFunction = selfControllerCalculateFrame;
		grabable->OnReleaseFunction = selfControllerOnRelease;
		rigid->grabable = grabable;
		selfController->addComponent(grabable);
		xrOriginObject.lock()->transform->addChild(selfController->transform);

		hd::GameObject selfControllerModel = std::make_shared<GameObject>();
		selfControllerModel->transform->setLocalScale({ 0.1f, 0.1f, 0.1f });
		selfControllerModel->models.push_back(whiteSphere);
		selfController->transform->addChild(selfControllerModel->transform);

		scene->addGameObject(selfController);
		scene->addGameObject(selfControllerModel);
		rigid->switchGravity(false);
		
	}

	sceneManager->Load(scene);
	sceneManager->Mobilize(scene);
}
