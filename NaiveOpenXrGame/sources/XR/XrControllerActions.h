#pragma once

#include "mainCommon.h"
#include "OpenXrInstance.h"
#include "Bricks/GameComponent.h"

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

		bool primaryButtonClicked();
		bool primaryButtonReleased();
		bool primaryButtonValue();

		bool secondaryButtonClicked();
		bool secondaryButtonReleased();
		bool secondaryButtonValue();

		glm::vec2 primaryAxisValue();

		void vibrate(const xr::Duration& duration = xr::Duration::minHaptic(), float frequency = XR_FREQUENCY_UNSPECIFIED, float amplitude = 1.f);

	private:
		int hand;
		bool m_triggerClicked = false;
		bool m_triggerReleased = false;
		bool triggerReleasing = true;
		bool m_gripClicked = false;
		bool m_gripReleased = false;
		bool gripReleasing = true;
		bool m_primaryButtonClicked = false;
		bool m_primaryButtonReleased = false;
		bool primaryButtonReleasing = true;
		bool m_secondaryButtonClicked = false;
		bool m_secondaryButtonReleased = false;
		bool secondaryButtonReleasing = true;

		rf::OpenXrInstance xrInstance;
	};
}

