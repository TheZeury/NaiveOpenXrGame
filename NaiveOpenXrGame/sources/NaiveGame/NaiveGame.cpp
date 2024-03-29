#include "NaiveGame.h"
#include "Physics/RigidStatic.h"
#include "Physics/ITriggerCallback.h"
#include "XR/XrControllerActions.h"
#include "XR/XrGrabber.h"
#include "XR/XrGrabable.h"
#include "NaiveGame/MachineGear.h"
#include "Renderer/MeshModel.h"
#include "Renderer/CharacterBitmap.h"
#include "Renderer/TextModel.h"
#include "Renderer/UIElement.h"
#include "XR/XrPointer.h"
#include "UI/Slider.h"

import NoxgMath;

Noxg::rf::GameObject rightHandObject;
Noxg::rf::GameObject leftHandObject;

Noxg::rf::GameObject selfControllerObject;
Noxg::rf::RigidDynamic selfControllerRigid;

Noxg::hd::XrControllerActions rightHandAction;
Noxg::hd::XrControllerActions leftHandAction;

struct SpaceSlider : public Noxg::Slider<float> {
	std::tuple<glm::vec3, glm::vec3> ends;
	Noxg::rf::ITransform transform;
	auto updatePosition() -> void override
	{
		auto pos = glm::mix(std::get<0>(ends), std::get<1>(ends), rawValue);
		transform.lock()->setLocalPosition(pos);
	}
	SpaceSlider(std::tuple<glm::vec3, glm::vec3> ends = { }, Noxg::rf::ITransform transform = { }) : ends(ends), transform(transform) { }
} valueControl;

glm::vec3 enterPos;

//Noxg::hd::GameObject leftHandBox;
//Noxg::hd::MachineGear leftHandGear;

auto createModelObjectFromPhysicsShape(PxShape& shape, Noxg::hd::Material material) -> Noxg::hd::GameObject
{
	using namespace Noxg;

	auto object = std::make_shared<GameObject>();
	object->transform->setLocalMatrix(cnv<glm::mat4>(PxMat44(shape.getLocalPose())));

	switch (shape.getGeometryType())
	{
	case PxGeometryType::eBOX:
	{
		PxBoxGeometry geometry;
		shape.getBoxGeometry(geometry);
		auto mesh = MeshBuilder::Box(geometry.halfExtents.x, geometry.halfExtents.y, geometry.halfExtents.z);
		object->addModel(mesh.build(material));
		break;
	}
	case PxGeometryType::eSPHERE:
	{
		PxSphereGeometry geometry;
		shape.getSphereGeometry(geometry);
		auto mesh = MeshBuilder::Icosphere(geometry.radius, 3);
		object->addModel(mesh.build(material));
		break;
	}
	case PxGeometryType::ePLANE:
	{
		break;
	}
	default:
	{
		break;
	}
	}

	return object;
}

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
	neutrGray = std::make_shared<Material>(glm::vec4{ .4f, .4f, .4f, 1.f });
	limeGreen = std::make_shared<Material>(glm::vec4{ .2f, .8f, .2f, 1.f });
	cyanBlue  = std::make_shared<Material>(glm::vec4{ 0.f, .7f, .9f, 1.f });
	
	MeshBuilder cubeBuilder = MeshBuilder::Box(0.5f, 0.5f, 0.5f);
	blackCube = cubeBuilder.build(pureBlack);
	whiteCube = cubeBuilder.build(neutrGray);
	MeshBuilder sphereBuilder = MeshBuilder::Icosphere(0.5f, 3);
	blackSphere = sphereBuilder.build(pureBlack);
	whiteSphere = sphereBuilder.build(neutrGray);

	bulletShape = physicsEngineInstance->createShape(PxSphereGeometry(0.05f), NaiveGameSimulationFilters::CommonInWorld);

	BuildScene();

	std::atomic<bool> mainLoopDone{ false };
	LOG_INFO("Game", "Entering into Main Loop.", 0);
	std::jthread main{ [&](std::stop_token st) { mainLoop(st); mainLoopDone = true; } };
	LOG_INFO("Game", "Entering into Fixed Loop.", 0);
	std::jthread fixed{ [&](std::stop_token st) { fixedLoop(st); } };
	
#ifdef MIRROR_WINDOW
	auto window = std::static_pointer_cast<VulkanInstance>(graphicsInstance)->getWindow();
	while (!mainLoopDone) 
	{
		if (glfwWindowShouldClose(window))
		{
			main.request_stop();
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		glfwPollEvents();
	}
#endif

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
	neutrGray = nullptr;
	limeGreen = nullptr;
	cyanBlue = nullptr;
	scene = nullptr;

	graphicsInstance->CleanUpSession();
	xrInstance->CleanUpSession();
	xrInstance->CleanUpInstance();
	graphicsInstance->CleanUpInstance();
}

void Noxg::NaiveGame::mainLoop(std::stop_token st)
{
	auto startTime = std::chrono::high_resolution_clock::now();
	auto lastTime = startTime;

	while (!st.stop_requested() && xrInstance->PollEvents())
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
				boxModel->addModel(leftHandBox->models[0]);
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
				//LOG_INFO("Game", "10 seconds of fixed loop.", 0);
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
	auto physxDebugScene = std::make_shared<Scene>();
	sceneManager->Initialize(scene);
	sceneManager->Initialize(physxDebugScene);
	scene->debugScene = physxDebugScene;
	scene->debugType = DebugType::eMixed;

	// XR Origin
	{
		auto origin = std::make_shared<GameObject>();
		auto camera = std::make_shared<GameObject>();
		origin->transform->addChild(camera->transform);
		
		scene->addGameObject(origin);
		scene->addGameObject(camera);

		xrOriginObject = origin;
		cameraObject = camera;
		scene->cameraTransform = camera->transform;
		physxDebugScene->cameraTransform = camera->transform;
	}

	// Ground.
	{
		auto tiles_diffuse = std::make_shared<Texture>("textures/GroundTile_diffuse.jpg");
		auto tiles_normal = std::make_shared<Texture>("textures/GroundTile_normal.jpg");
		auto tiles = std::make_shared<Material>(tiles_diffuse, tiles_normal);
		std::vector<Vertex> vertices = {
			{ { -100.f, 0.f,  100.f }, { 100.f,   0.f }, { 0.f, 1.f, 0.f }, { }, { } },
			{ {  100.f, 0.f,  100.f }, {   0.f,   0.f }, { 0.f, 1.f, 0.f }, { }, { } },
			{ {  100.f, 0.f, -100.f }, {   0.f, 100.f }, { 0.f, 1.f, 0.f }, { }, { } },
			{ { -100.f, 0.f, -100.f }, { 100.f, 100.f }, { 0.f, 1.f, 0.f }, { }, { } },
		};
		std::vector<uint32_t> indices = {
			0, 1, 2,
			2, 3, 0,
		};
		auto groundModel = std::make_shared<MeshModel>(vertices, indices, tiles);
		auto ground = std::make_shared<GameObject>();
		ground->addModel(groundModel);

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

	// Right hand.
	{
		hd::GameObject rightHand = std::make_shared<GameObject>();
		if (!xrOriginObject.expired())
		{
			xrOriginObject.lock()->transform->addChild(rightHand->transform);
		}
		rightHandAction = std::make_shared<XrControllerActions>(1);
		rightHand->addComponent(rightHandAction);

		hd::XrGrabber grabber = std::make_shared<XrGrabber>(rightHandAction, PxBoxGeometry(0.1f, 0.1f, 0.1f));
		rightHand->addComponent(grabber);

		hd::XrPointer pointer = std::make_shared<XrPointer>(rightHandAction, PxSphereGeometry(0.01f), physicsEngineInstance);
		rightHand->addComponent(pointer);

		auto shape = pointer->fetchShape();

		auto triggerObject = std::make_shared<GameObject>();
		triggerObject->transform = std::make_shared<PhysicsTransform>(nullptr);
		hd::RigidStatic rigid = std::make_shared<RigidStatic>();
		rigid->addShape(shape);
		triggerObject->addComponent(rigid);
		rightHand->transform->addChild(triggerObject->transform);

		auto whiteCone = MeshBuilder::Cone(0.1f, 0.5f, 1.f, 16).build(neutrGray);

		hd::GameObject rightHandModel = std::make_shared<GameObject>();
		rightHandModel->transform->setLocalScale({ 0.1f, 0.1f, 0.1f });
		rightHandModel->addModel(whiteCone);
		rightHand->transform->addChild(rightHandModel->transform);

		hd::GameObject pointerModel = createModelObjectFromPhysicsShape(*shape, cyanBlue);
		rightHand->transform->addChild(pointerModel->transform);

		rightHandObject = rightHand;
		scene->addGameObject(rightHand);
		scene->addGameObject(triggerObject);
		scene->addGameObject(rightHandModel);
		physxDebugScene->addGameObject(pointerModel);
	}

	// Left hand.
	{
		hd::GameObject leftHand = std::make_shared<GameObject>();
		if (!xrOriginObject.expired())
		{
			xrOriginObject.lock()->transform->addChild(leftHand->transform);
		}
		leftHandAction = std::make_shared<XrControllerActions>(0);
		leftHand->addComponent(leftHandAction);

		hd::XrGrabber grabber = std::make_shared<XrGrabber>(leftHandAction, PxBoxGeometry(0.1f, 0.1f, 0.1f));
		leftHand->addComponent(grabber);

		hd::XrPointer pointer = std::make_shared<XrPointer>(leftHandAction, PxSphereGeometry(0.01f), physicsEngineInstance);
		leftHand->addComponent(pointer);

		auto shape = pointer->fetchShape();

		auto triggerObject = std::make_shared<GameObject>();
		triggerObject->transform = std::make_shared<PhysicsTransform>(nullptr);
		hd::RigidStatic rigid = std::make_shared<RigidStatic>();
		rigid->addShape(shape);
		triggerObject->addComponent(rigid);
		leftHand->transform->addChild(triggerObject->transform);

		auto blackCone = MeshBuilder::Cone(0.1f, 0.5f, 1.f, 16).build(pureBlack);

		hd::GameObject leftHandModel = std::make_shared<GameObject>();
		leftHandModel->transform->setLocalScale({ 0.1f, 0.1f, 0.1f });
		leftHandModel->addModel(blackCone);
		leftHand->transform->addChild(leftHandModel->transform);

		hd::GameObject pointerModel = createModelObjectFromPhysicsShape(*shape, cyanBlue);
		leftHand->transform->addChild(pointerModel->transform);

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
		scene->addGameObject(triggerObject);
		scene->addGameObject(leftHandModel);
		physxDebugScene->addGameObject(pointerModel);
	}

	// Steed.
	{
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

	// Cube
	{
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
		boxModel->addModel(whiteCube);
		box->transform->addChild(boxModel->transform);

		auto shapeModel = createModelObjectFromPhysicsShape(*shape, limeGreen);
		box->transform->addChild(shapeModel->transform);

		scene->addGameObject(box);
		scene->addGameObject(boxModel);

		physxDebugScene->addGameObject(shapeModel);
	}

	// Sphere
	{
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
		sphereModel->addModel(whiteSphere);
		sphere->transform->addChild(sphereModel->transform);

		auto shapeModel = createModelObjectFromPhysicsShape(*shape, limeGreen);
		sphere->transform->addChild(shapeModel->transform);

		scene->addGameObject(sphere);
		scene->addGameObject(sphereModel);

		physxDebugScene->addGameObject(shapeModel);
	}

	// Cone
	{
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

		auto whiteCone = MeshBuilder::Cone(0.5f, 0.2f, 1.f, 12).build(neutrGray);

		hd::GameObject coneModel = std::make_shared<GameObject>();
		coneModel->addModel(whiteCone);
		cone->transform->addChild(coneModel->transform);

		auto shapeModel = createModelObjectFromPhysicsShape(*shape, limeGreen);
		cone->transform->addChild(shapeModel->transform);

		scene->addGameObject(cone);
		scene->addGameObject(coneModel);

		physxDebugScene->addGameObject(shapeModel);
	}

	// Wooden Table
	{
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

		auto shapeModel = createModelObjectFromPhysicsShape(*shape, limeGreen);
		table->transform->addChild(shapeModel->transform);

		scene->addGameObject(table);
		scene->addGameObject(tableModel);

		physxDebugScene->addGameObject(shapeModel);
	}

	// Revolver
	{
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
				bulletModel->addModel(blackSphere);
				bulletModel->transform->setLocalScale({ 0.1f, 0.1f, 0.1f });
				bullet->transform->addChild(bulletModel->transform);

				auto shapeModel = createModelObjectFromPhysicsShape(*bulletShape, limeGreen);
				bullet->transform->addChild(shapeModel->transform);

				scene->addGameObject(bullet);
				scene->addGameObject(bulletModel);

				scene->debugScene->addGameObject(shapeModel);
			}
		};
		grabable->GrabbingFrameCalculateFunction = RevolverCalculateFrame;
		rigid->grabable = grabable;
		revolver->addComponent(grabable);

		auto pointable = std::make_shared<XrPointable>();
		auto OnEnter = [](hd::XrControllerActions controller)
		{
			controller->vibrate();
		}; 
		auto OnExit = [](hd::XrControllerActions controller)
		{
			controller->vibrate();
		};
		pointable->OnEnterFunction = OnEnter;
		pointable->OnExitFunction = OnExit;
		rigid->pointable = pointable;
		revolver->addComponent(pointable);

		hd::GameObject revolverModel = graphicsInstance->loadGameObjectFromFiles("revolver");
		revolverModel->transform->setLocalPosition({ 0.0f, 0.f, 0.01f });
		revolverModel->transform->setLocalScale({ 2.54f, 2.54f, 2.54f });	// 0.01 inch to m
		revolver->transform->addChild(revolverModel->transform);

		auto shapeModel = createModelObjectFromPhysicsShape(*shape, limeGreen);
		revolver->transform->addChild(shapeModel->transform);

		scene->addGameObject(revolver);
		scene->addGameObject(revolverModel);

		physxDebugScene->addGameObject(shapeModel);
	}

	// Self Controller ( Provide movement )
	{
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
		selfControllerModel->addModel(whiteSphere);
		selfController->transform->addChild(selfControllerModel->transform);

		auto shapeModel = createModelObjectFromPhysicsShape(*shape, cyanBlue);
		selfController->transform->addChild(shapeModel->transform);

		scene->addGameObject(selfController);
		scene->addGameObject(selfControllerModel);
		rigid->switchGravity(false);

		physxDebugScene->addGameObject(shapeModel);
		
	}

	// Poem
	{
		auto poem = std::make_shared<GameObject>();
		poem->transform->setLocalPosition({ -20.f, 20.f, -20.f });
		auto bitmap = std::make_shared<CharacterBitmap>("fonts/Anonymous_Pro.ttf");
		std::string text = "I wandered lonely as a cloud\nThat floats on high o'er vales and hills,\nWhen all at once I saw a crowd,\nA host, of golden daffodils;\nBeside the lake, beneath the trees,\nFluttering and dancing in the breeze.\n";
		text += "\nIt was the best of times, it was the worst of times,\nit was the age of wisdom, it was the age of foolishness,\nit was the epoch of belief, it was the epoch of incredulity,\nit was the season of Light, it was the season of Darkness,\nit was the spring of hope, it was the winter of despair,";
		auto texture = std::make_shared<Texture>("textures/robert-lukeman-PH0HYjsf2n8-unsplash.jpg");
		auto material = std::make_shared<Material>(texture);
		auto poemText = std::make_shared<TextModel>(text, material, bitmap, 1.5f);
		poem->addText(poemText);

		scene->addGameObject(poem);
	}

	// Greeting Panel
	{
		auto panelObject = std::make_shared<GameObject>();
		panelObject->transform->setLocalPosition({ 0.f, 0.7f, -0.3f });
		panelObject->transform->setLocalRotation({ 0.9239f, -0.3827f, 0.f, 0.f });
		auto pannel = UIElement::PanelElement({ 0.2f, 0.3f }, std::make_shared<Texture>("textures/robert-lukeman-PH0HYjsf2n8-unsplash.jpg"));
		panelObject->addUIElement(pannel);

		auto greetingObject = std::make_shared<GameObject>();
		greetingObject->transform->setLocalPosition({ 0.f, 0.f, 0.001f });
		auto greeting = UIElement::TextElement("Hello UI", 0.03f, std::make_shared<CharacterBitmap>("fonts/Anonymous_Pro.ttf"));
		greetingObject->addUIElement(greeting);
		panelObject->transform->addChild(greetingObject->transform);

		scene->addGameObject(panelObject);
		scene->addGameObject(greetingObject);

		// Slider
		{
			auto sliderParent = std::make_shared<GameObject>();
			panelObject->transform->addChild(sliderParent->transform);
			auto slider = std::make_shared<GameObject>();
			auto model = MeshBuilder::Icosphere(0.01f, 3).build(pureWhite);
			slider->addModel(model);

			auto rigid = std::make_shared<RigidDynamic>();
			slider->transform = std::make_shared<PhysicsTransform>(nullptr);
			slider->transform->setLocalPosition({ 0.f, -0.03f, 0.01f });
			sliderParent->transform->addChild(slider->transform);
			auto shape = physicsEngineInstance->createShape(PxSphereGeometry(0.01f), NaiveGameSimulationFilters::CommonUIHovering);
			rigid->addShape(shape);
			slider->addComponent(rigid);

			valueControl = SpaceSlider{ std::tuple(glm::vec3{ -0.1f, -0.03f, 0.01f }, glm::vec3{ 0.1f, -0.03f, 0.01f }), slider->transform };
			valueControl.setRaw(0.5f);

			auto pointable = std::make_shared<XrPointable>();
			auto OnPointed = [&](hd::XrControllerActions controller)
			{
				enterPos = controller->gameObject.lock()->transform->getLocalPosition() + ((std::get<0>(valueControl.ends) - std::get<1>(valueControl.ends)) * valueControl.getRaw());
			};
			auto frameCalculation = [&](hd::XrControllerActions controller)
			{
				auto value = std::clamp((controller->gameObject.lock()->transform->getLocalPosition() - enterPos).x, 0.0f, 0.2f) * 5.f;
				valueControl.setValue(value);
			};
			pointable->OnPointFunction = OnPointed;
			pointable->PointingFrameCalculateFunction = frameCalculation;
			rigid->pointable = pointable;
			slider->addComponent(pointable);

			auto stickObject = std::make_shared<GameObject>();
			stickObject->transform->setLocalPosition({ 0.f,  -0.03f, 0.01f });
			stickObject->transform->setLocalRotation(cnv<glm::quat>(PxShortestRotation({ 0.f, 1.f, 0.f }, { 1.f, 0.f, 0.f })));
			stickObject->addModel(MeshBuilder::Cone(0.002, 0.002, 0.2, 6).build(neutrGray));
			sliderParent->transform->addChild(stickObject->transform);

			auto shapeModel = createModelObjectFromPhysicsShape(*shape, cyanBlue);
			slider->transform->addChild(shapeModel->transform);


			scene->addGameObject(sliderParent);
			scene->addGameObject(slider);
			scene->addGameObject(stickObject);
			rigid->switchGravity(false);

			physxDebugScene->addGameObject(shapeModel);
		}

		// Button(Collider On/Off)
		{
			auto buttonParent = std::make_shared<GameObject>();
			buttonParent->transform->setLocalPosition({ 0.f, -0.09f, 0.f });
			panelObject->transform->addChild(buttonParent->transform);
			auto button = std::make_shared<GameObject>();
			auto model = MeshBuilder::Box(0.03f, 0.01f, 0.005f).build(pureWhite);
			button->addModel(model);
			button->transform = std::make_shared<PhysicsTransform>(nullptr);
			button->transform->setLocalPosition({ 0.f, 0.f, 0.005f });
			buttonParent->transform->addChild(button->transform);

			auto rigid = std::make_shared<RigidDynamic>();
			auto shape = physicsEngineInstance->createShape(PxBoxGeometry(0.03f, 0.01f, 0.005f), NaiveGameSimulationFilters::CommonUIHovering);
			rigid->addShape(shape);
			button->addComponent(rigid);

			auto pointable = std::make_shared<XrPointable>();
			auto OnPointed = [&](hd::XrControllerActions controller)
			{
				++scene->debugType;
			};
			pointable->OnPointFunction = OnPointed;
			rigid->pointable = pointable;
			button->addComponent(pointable);

			auto shapeModel = createModelObjectFromPhysicsShape(*shape, cyanBlue);
			button->transform->addChild(shapeModel->transform);

			auto textObject = std::make_shared<GameObject>();
			textObject->transform->setLocalPosition({ 0.f, 0.f, 0.011f });
			auto text = UIElement::TextElement("Collider", 0.01f, std::make_shared<CharacterBitmap>("fonts/Anonymous_Pro.ttf"));
			textObject->addUIElement(text);
			buttonParent->transform->addChild(textObject->transform);

			scene->addGameObject(buttonParent);
			scene->addGameObject(button);
			scene->addGameObject(textObject);
			rigid->switchGravity(false);

			physxDebugScene->addGameObject(shapeModel);
		}

	}

	/*/ Button(Collider On/Off)
	{
		auto buttonParent = std::make_shared<GameObject>();
		buttonParent->transform->setLocalPosition({ 0.f, 0.7f, -0.3f });
		buttonParent->transform->setLocalRotation({ 0.9239f, -0.3827f, 0.f, 0.f });
		auto button = std::make_shared<GameObject>();
		auto model = MeshBuilder::Box(0.03f, 0.01f, 0.005f).build(pureWhite);
		button->addModel(model);
		button->transform = std::make_shared<PhysicsTransform>(nullptr);
		button->transform->setLocalPosition({ 0.f, -0.09f, 0.005f });
		buttonParent->transform->addChild(button->transform);
		
		auto rigid = std::make_shared<RigidDynamic>();
		auto shape = physicsEngineInstance->createShape(PxBoxGeometry(0.03f, 0.01f, 0.005f), NaiveGameSimulationFilters::CommonUIHovering);
		rigid->addShape(shape);
		button->addComponent(rigid);

		auto pointable = std::make_shared<XrPointable>();
		auto OnPointed = [&](hd::XrControllerActions controller)
		{
			++scene->debugType;
		};
		pointable->OnPointFunction = OnPointed;
		rigid->pointable = pointable;
		button->addComponent(pointable);

		auto shapeModel = createModelObjectFromPhysicsShape(*shape, cyanBlue);
		button->transform->addChild(shapeModel->transform);

		scene->addGameObject(buttonParent);
		scene->addGameObject(button);
		rigid->switchGravity(false);

		physxDebugScene->addGameObject(shapeModel);
	}*/
	
	sceneManager->Load(scene);
	sceneManager->Mobilize(scene);
}
