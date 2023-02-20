#pragma once

#include "mainCommon.h"

namespace Noxg
{
	MAKE_HANDLE(ITriggerCallback);

	class ITriggerCallback
	{
	public:
		virtual void OnEnter(const PxTriggerPair& pair) = 0;
		virtual void OnExit(const PxTriggerPair& pair) = 0;
	};
}

