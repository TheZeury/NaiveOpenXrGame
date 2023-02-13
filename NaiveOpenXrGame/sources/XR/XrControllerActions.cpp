#include "XrControllerActions.h"
#include "Bricks/SceneManager.h"

Noxg::XrControllerActions::XrControllerActions(int handIndex) : hand{ handIndex }
{
	
}

void Noxg::XrControllerActions::Enable()
{
	xrInstance = std::dynamic_pointer_cast<OpenXrInstance>(gameObject.lock()->scene.lock()->manager.lock()->defaultXrInstance.lock());
}

void Noxg::XrControllerActions::CalculateFrame()
{
	{	// Trigger state.
		m_triggerClicked = false;
		m_triggerReleased = false;
		float value = triggerValue();
		if (triggerReleasing)	// releasing at last frame.
		{
			if (value > 0.7)	// not releasing now.
			{
				m_triggerClicked = true;
				triggerReleasing = false;
			}
			// else just releasing.
		}
		else	// not releasing at last frame.
		{
			if (value < 0.3)	// releasing now.
			{
				m_triggerReleased = true;
				triggerReleasing = true;
			}
			// else just not releasing.
		}
		// releasing state for value in [0.3, 0.7] is based on last change.
	}

	{	// Grip state.
		m_gripClicked = false;
		m_gripReleased = false;
		float value = gripValue();
		if (gripReleasing)
		{
			if (value > 0.3)
			{
				m_gripClicked = true;
				gripReleasing = false;
			}
		}
		else
		{
			if (value <= 0.3)
			{
				m_gripReleased = true;
				gripReleasing = true;
			}
		}
	}

	{	// Primary button state.
		m_primaryButtonClicked = false;
		m_primaryButtonReleased = false;
		bool value = primaryButtonValue();
		if (primaryButtonReleasing)
		{
			if (value)
			{
				m_primaryButtonClicked = true;
				primaryButtonReleasing = false;
			}
		}
		else
		{
			if (!value)
			{
				m_primaryButtonReleased = true;
				primaryButtonReleasing = true;
			}
		}
	}

	{	// Secondary button state.
		m_secondaryButtonClicked = false;
		m_secondaryButtonReleased = false;
		bool value = secondaryButtonValue();
		if (secondaryButtonReleasing)
		{
			if (value)
			{
				m_secondaryButtonClicked = true;
				secondaryButtonReleasing = false;
			}
		}
		else
		{
			if (!value)
			{
				m_secondaryButtonReleased = true;
				secondaryButtonReleasing = true;
			}
		}
	}

	{	// Controller pose.
		gameObject.lock()->transform->setLocalPosition(position());
		gameObject.lock()->transform->setLocalRotation(rotation());
	}
}

bool Noxg::XrControllerActions::isActive()
{
	auto xr = xrInstance.lock();
	if (xr == nullptr) return false;
	return static_cast<bool>(xr->inputState.handActive[hand]);
}

glm::vec3 Noxg::XrControllerActions::position()
{
	auto xr = xrInstance.lock();
	if (xr == nullptr) return { };
	return *((glm::vec3*)(&(xr->inputState.handLocations[hand].pose.position)));
}

glm::quat Noxg::XrControllerActions::rotation()
{
	auto xr = xrInstance.lock();
	if (xr == nullptr) return { 1.f, 0.f, 0.f, 0.f };
	return *((glm::quat*)(&(xr->inputState.handLocations[hand].pose.orientation)));
}

bool Noxg::XrControllerActions::triggerClicked()
{
	return m_triggerClicked;
}

bool Noxg::XrControllerActions::triggerReleased()
{
	return m_triggerReleased;
}

float Noxg::XrControllerActions::triggerValue()
{
	auto xr = xrInstance.lock();
	if (xr == nullptr) return 0.0f;
	return xr->inputState.triggerStates[hand].currentState;
}

bool Noxg::XrControllerActions::gripClicked()
{
	return m_gripClicked;
}

bool Noxg::XrControllerActions::gripReleased()
{
	return m_gripReleased;
}

float Noxg::XrControllerActions::gripValue()
{
	auto xr = xrInstance.lock();
	if (xr == nullptr) return 0.0f;
	return xr->inputState.gripStates[hand].currentState;
}

bool Noxg::XrControllerActions::primaryButtonClicked()
{
	return m_primaryButtonClicked;
}

bool Noxg::XrControllerActions::primaryButtonReleased()
{
	return m_primaryButtonReleased;
}

bool Noxg::XrControllerActions::primaryButtonValue()
{
	auto xr = xrInstance.lock();
	if (xr == nullptr) return false;
	return static_cast<bool>(xr->inputState.primaryButtonStates[hand].currentState);
}

bool Noxg::XrControllerActions::secondaryButtonClicked()
{
	return m_secondaryButtonClicked;
}

bool Noxg::XrControllerActions::secondaryButtonReleased()
{
	return m_secondaryButtonReleased;
}

bool Noxg::XrControllerActions::secondaryButtonValue()
{
	auto xr = xrInstance.lock();
	if (xr == nullptr) return false;
	return static_cast<bool>(xr->inputState.secondaryButtonStates[hand].currentState);
}

glm::vec2 Noxg::XrControllerActions::primaryAxisValue()
{
	auto xr = xrInstance.lock();
	if (xr == nullptr) return { };
	return { xr->inputState.thumbstickXStates[hand].currentState, xr->inputState.thumbstickYStates[hand].currentState };
}
