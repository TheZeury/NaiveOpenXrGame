#pragma once

#include "mainCommon.h"
#include "Bricks/GameComponent.h"
#include "XrControllerActions.h"

namespace Noxg
{
	MAKE_HANDLE(XrPointable);
	
	class XrPointable : public GameComponent
	{
	public:
		virtual void Enable() override { }
		rf::XrControllerActions controller;

		std::function<void(hd::XrControllerActions)> OnEnterFunction;
		std::function<void(hd::XrControllerActions)> OnExitFunction;
		std::function<void(hd::XrControllerActions)> OnPointFunction;
		std::function<void(hd::XrControllerActions)> OnReleaseFunction;
		std::function<void(hd::XrControllerActions)> EnteringFrameCalculateFunction;
		std::function<void(hd::XrControllerActions)> PointingFrameCalculateFunction;

		void virtual OnEnter();
		void virtual OnExit();
		void virtual OnPoint();
		void virtual OnRelease();
		void virtual EnteringFrameCalculate();
		void virtual PointingFrameCalculate();
	};
}
