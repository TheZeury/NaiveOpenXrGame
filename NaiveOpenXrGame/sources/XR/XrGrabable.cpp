#include "XrGrabable.h"

void Noxg::XrGrabable::OnGrab()
{
	if(freeGrabbing)
	{
		auto controllerTransform = controller.lock()->gameObject.lock()->transform->getGlobalMatrix();
		auto objectTransform = gameObject.lock()->transform->getGlobalMatrix();
		XrMatrix4x4f invController;
		XrMatrix4x4f_InvertRigidBody(&invController, (XrMatrix4x4f*)(&controllerTransform));
		XrMatrix4x4f_Multiply((XrMatrix4x4f*)(&attachTransformation), &invController, (XrMatrix4x4f*)(&objectTransform));
	}
	if(OnGrabFunction != nullptr)
	{
		OnGrabFunction(controller.lock());
	}
}

void Noxg::XrGrabable::OnRelease()
{
	if (OnReleaseFunction != nullptr)
	{
		OnReleaseFunction(controller.lock());
	}
}

void Noxg::XrGrabable::GrabbingFrameCalculate()
{
	if (GrabbingFrameCalculateFunction != nullptr)
	{
		GrabbingFrameCalculateFunction(controller.lock());
	}
}
