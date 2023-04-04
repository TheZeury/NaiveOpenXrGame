#pragma once

#include "mainCommon.h"
#include "XrControllerActions.h"
#include "XR/XrPointable.h"
#include "Physics/PhysicsEngineInstance.h"

namespace Noxg
{
	MAKE_HANDLE(XrPointer);

	class XrPointer : public GameComponent, public IHaveFrameCalculation
	{
	public:
		XrPointer() = delete;
		XrPointer(rf::XrControllerActions controller, const PxSphereGeometry& shape, rf::PhysicsEngineInstance physics);

		virtual void Enable() override;
		virtual void CalculateFrame() override;

		rf::XrPointable pointedPointable;
		rf::XrControllerActions controller;
		const PxSphereGeometry& shapeGeometry;
		PxShape* shape;
		std::unordered_set<PxShape*> enteredShapes;

	public:
		auto fetchShape() -> PxShape*;

	private:
		rf::XrPointable getPointableObject();
		rf::PhysicsEngineInstance physics;
	};
}

