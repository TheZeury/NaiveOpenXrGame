#pragma once

#include "mainCommon.h"
#include "Bricks/GameComponent.h"
#include "Bricks/Transform.h"
#include "XrControllerActions.h"

namespace Noxg
{
	MAKE_HANDLE(XrGrabable);

	class XrGrabable : public GameComponent
	{
	public:
		void virtual Enable() override;
		bool freeGrabbing = true;
		glm::mat4 attachTransformation;	// attachTransform is the transform of this object in `controller space`, so `controllerTransform * attachTransformation = objectTransform`.
		rf::XrControllerActions controller;	// Current controller. Set by XrGrabber.

		void virtual OnGrab();
		void virtual OnRelease();
		// This function will be called by XrGrabber every frame when this Grabable is being Grabbed.
		void virtual GrabbingFrameCalculate();
	};
}

