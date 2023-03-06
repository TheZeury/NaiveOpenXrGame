#pragma once

#include "mainCommon.h"
#include "Bricks/GameComponent.h"
#include "XrControllerActions.h"
#include "Physics/RigidDynamic.h"

namespace Noxg
{
	MAKE_HANDLE(XrGrabber);

	class XrGrabber : public GameComponent, public IHaveFrameCalculation
	{
	public:
		XrGrabber() = delete;
		XrGrabber(rf::XrControllerActions controller, const PxSphereGeometry& shape);
		XrGrabber(rf::XrControllerActions controller, const PxBoxGeometry& shape);

		virtual void Enable() override;
		virtual void CalculateFrame() override;

		bool grabbing = false;
		rf::GameObject grabbedGameObject;
		rf::RigidDynamic rigid;
		rf::XrControllerActions controller;
		std::shared_ptr<PxGeometry> shape;
	};
}

