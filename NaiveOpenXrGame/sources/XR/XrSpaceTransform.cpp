#include "XrSpaceTransform.h"

void Noxg::XrSpaceTransform::CalculateFrame()
{
	localPosition = *((glm::vec3*)(&(spaceLocation.pose.position)));
	localRotation = *((glm::quat*)(&(spaceLocation.pose.orientation)));
	localChanged = true;
	globalChanged = true;
	getLocalMatrix();	// getLocalMatrix will calculate the new transform.
}
