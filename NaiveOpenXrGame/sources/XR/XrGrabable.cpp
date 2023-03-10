#include "XrGrabable.h"

void Noxg::XrGrabable::Enable()
{
	attachTransformation = glm::mat4{ 1.f };
}

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
}

void Noxg::XrGrabable::OnRelease()
{
	LOG_INFO("Game", "Released.", 0);
}

void Noxg::XrGrabable::GrabbingFrameCalculate()
{
}
