#pragma once

#include "mainCommon.h"
#include "Bricks/Transform.h"

namespace Noxg
{
	MAKE_HANDLE(XrSpaceTransform);

	class XrSpaceTransform : public Transform, public IHaveFrameCalculation
	{
	public:
		XrSpaceTransform(xr::SpaceLocation& space) : spaceLocation(space) { }

	public:
		virtual void CalculateFrame() override;

	private:
		rf::ITransform attach;
		xr::SpaceLocation& spaceLocation;
	};
}
