#include "XrPointer.h"
#include "Physics/ITriggerCallback.h"
#include "Physics/RigidActor.h"

class XrPointerTriggerCallback : public Noxg::ITriggerCallback
{
public:
	virtual void OnEnter(const PxTriggerPair& pair) override 
	{
		auto rigid = reinterpret_cast<Noxg::rf::RigidActor*>(pair.otherActor->userData);
		if (!rigid->expired())
		{
			auto pointable = rigid->lock()->pointable;
			if (!pointable.expired())
			{
				pointable.lock()->controller = controller;
				pointable.lock()->OnEnter();
				enteredShapes.insert(pair.otherShape);
				LOG_INFO("PhysX", std::format("Trigger Entered, total shapes: {}", enteredShapes.size()), 0);
			}
		}
	};
	virtual void OnExit(const PxTriggerPair& pair) override 
	{ 
		if(enteredShapes.count(pair.otherShape) != 0)
		{
			auto pointable = reinterpret_cast<Noxg::rf::RigidActor*>(pair.otherActor->userData)->lock()->pointable.lock();
			pointable->OnExit();
			// pointable->controller.reset(); // Don't reset. It's possible that another controller is pointing at the same object.
			enteredShapes.erase(pair.otherShape);
			LOG_INFO("PhysX", std::format("Trigger Exited, total shapes: {}", enteredShapes.size()), 0);
		}
	};

	std::unordered_set<PxShape*>& enteredShapes;
	Noxg::rf::XrControllerActions controller;

	XrPointerTriggerCallback(std::unordered_set<PxShape*>& enteredShapes, Noxg::rf::XrControllerActions controller) : enteredShapes{ enteredShapes }, controller{ controller } { }
};

Noxg::XrPointer::XrPointer(rf::XrControllerActions controller, const PxSphereGeometry& geometry, rf::PhysicsEngineInstance physics) : controller{ controller }, shapeGeometry{ geometry }, physics{ physics }
{
	if (controller.expired()) LOG_ERRO("No controller!!!!!");
	shape = physics.lock()->createShape(shapeGeometry, NaiveGameSimulationFilters::CommonInWorld);
	shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
	shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
	shape->setLocalPose(PxTransform{ PxVec3{ 0.f, -0.05f, 0.f }, PxQuat{ PxIdentity } });
	ITriggerCallback* triggerCallback = new XrPointerTriggerCallback{ enteredShapes, this->controller };
	shape->userData = triggerCallback;
}

void Noxg::XrPointer::Enable()
{
	
}

void Noxg::XrPointer::CalculateFrame()
{
	if (controller.expired()) return;
	auto controller = this->controller.lock();

	if (!pointedPointable.expired())
	{
		if (controller->triggerReleased())
		{
			pointedPointable.lock()->OnRelease();
			pointedPointable.reset();
		}
		else
		{
			pointedPointable.lock()->PointingFrameCalculate();
		}
	}
	else
	{
		if (controller->triggerClicked())
		{
			auto pointedPointable = getPointableObject();
			if (!pointedPointable.expired())
			{
				pointedPointable.lock()->OnPoint();
				this->pointedPointable = pointedPointable;
			}
		}
	}
}

auto Noxg::XrPointer::fetchShape() -> PxShape*
{
	return shape;
}

Noxg::rf::XrPointable Noxg::XrPointer::getPointableObject()
{
	// return an arbitrary pointable object from the entered shapes.
	if (enteredShapes.size() == 0) return rf::XrPointable();
	auto shape = *enteredShapes.begin();
	auto rigid = reinterpret_cast<rf::RigidActor*>(shape->getActor()->userData);
	if (rigid->expired()) return rf::XrPointable();
	auto pointable = rigid->lock()->pointable;
	if (pointable.expired()) return rf::XrPointable();
	return pointable;
}
