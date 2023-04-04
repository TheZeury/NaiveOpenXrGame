#include "XrPointable.h"

void Noxg::XrPointable::OnEnter()
{
	if (OnEnterFunction != nullptr)
	{
		OnEnterFunction(controller.lock());
	}
}

void Noxg::XrPointable::OnExit()
{
	if (OnExitFunction != nullptr)
	{
		OnExitFunction(controller.lock());
	}
}

void Noxg::XrPointable::OnPoint()
{
	if (OnPointFunction != nullptr)
	{
		OnPointFunction(controller.lock());
	}
}

void Noxg::XrPointable::OnRelease()
{
	if (OnReleaseFunction != nullptr)
	{
		OnReleaseFunction(controller.lock());
	}
}

void Noxg::XrPointable::EnteringFrameCalculate()
{
	if (EnteringFrameCalculateFunction != nullptr)
	{
		EnteringFrameCalculateFunction(controller.lock());
	}
}

void Noxg::XrPointable::PointingFrameCalculate()
{
	if (PointingFrameCalculateFunction != nullptr)
	{
		PointingFrameCalculateFunction(controller.lock());
	}
}