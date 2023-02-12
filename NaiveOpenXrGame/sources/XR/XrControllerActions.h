#pragma once

#include "mainCommon.h"
#include "OpenXrInstance.h"
#include "Bricks/GameComponent.h"

#include <functional>

namespace Noxg
{
	MAKE_HANDLE(XrControllerActions);

	class XrControllerActions : public GameComponent, public IHaveFrameCalculation
	{
	public:
		XrControllerActions(int handIndex);

		virtual void Enable() override;

		virtual void CalculateFrame();

		bool isActive();
		glm::vec3 position();
		glm::quat rotation();

		bool triggerClicked();
		bool triggerReleased();
		float triggerValue();

		bool gripClicked();
		bool gripReleased();
		float gripValue();

		/*bool primaryButtonClicked();
		bool primaryButtonReleased();

		bool secondaryButtonClicked();
		bool secondaryButtonReleased();*/

		glm::vec2 primaryAxisValue();
	private:
		int hand;
		bool m_triggerClicked = false;
		bool m_triggerReleased = false;
		bool triggerReleasing = true;
		bool m_gripClicked = false;
		bool m_gripReleased = false;
		bool gripReleasing = true;

		rf::OpenXrInstance xrInstance;
	};
}

