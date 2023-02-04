#pragma once

#include "mainCommon.h"

namespace Noxg
{
	MAKE_HANDLE(XrInstance);

	class XrInstance
	{
	public:
		virtual void Initialize() = 0;
		virtual void CleanUpInstance() = 0;
		virtual void CleanUpSession() = 0;
		virtual void InitializeSession() = 0;

		virtual bool PollEvents() = 0;
		virtual bool running() = 0;
		virtual void PollActions() = 0;
		virtual void Update() = 0;
	public:
		virtual void vibrate(const xr::HapticVibration& virbation, int hand) = 0;
	};
}

