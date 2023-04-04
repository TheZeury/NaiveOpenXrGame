#include "XrGrabber.h"
#include "Physics/ITriggerCallback.h"
#include "XrGrabable.h"

Noxg::XrGrabber::XrGrabber(rf::XrControllerActions controller, const PxSphereGeometry& shape) : controller{ controller }, shape{ std::make_shared<PxSphereGeometry>(PxSphereGeometry(shape)) }
{
	
}

Noxg::XrGrabber::XrGrabber(rf::XrControllerActions controller, const PxBoxGeometry& shape) : controller{ controller }, shape{ std::make_shared<PxBoxGeometry>(PxBoxGeometry(shape)) }
{

}

void Noxg::XrGrabber::Enable()
{
	if (controller.expired()) 
	{
		LOG_ERRO("No valid controller detected or speicified when enabling a XrGrabber.");
	}
}

void Noxg::XrGrabber::CalculateFrame()
{
	if (controller.expired()) return;
	auto ctrlr = controller.lock();
	if (!grabbedGrabable.expired())
	{
		if (ctrlr->gripReleased())
		{
			// release.
			//rigid.lock()->addForce({ }, PxForceMode::eVELOCITY_CHANGE);
			rigid.lock()->switchGravity(true);
			grabbedGrabable.lock()->OnRelease();
			grabbedGrabable.reset();
		}
		else
		{
			// set position directly.
			rf::GameObject grabbedObject = grabbedGrabable.lock()->gameObject;
			grabbedObject.lock()->transform->setGlobalMatrix(gameObject.lock()->transform->getGlobalMatrix() * grabbedGrabable.lock()->attachTransformation);
			grabbedGrabable.lock()->GrabbingFrameCalculate();
			//rigid.lock()->addForce({ }, PxForceMode::eVELOCITY_CHANGE);

			// force based.
			//glm::vec3 force = gameObject.lock()->transform->getLocalPosition() - grabbedGameObject.lock()->transform->getLocalPosition();
			//rigid.lock()->setLinearVelocity(force / 0.02f);
		}
	}
	else
	{
		if (ctrlr->gripClicked())
		{
			PxOverlapBuffer overlapBuffer;
			const auto& matrix = gameObject.lock()->transform->getGlobalMatrix();
			PxTransform transform{ *((PxMat44*)(&matrix)) };
			gameObject.lock()->scene.lock()->physicsScene->overlap(PxBoxGeometry(0.05f, 0.05f, 0.05f), transform, overlapBuffer);
			if (overlapBuffer.hasAnyHits()) 
			{
				rigid = std::dynamic_pointer_cast<RigidDynamic>(reinterpret_cast<rf::RigidActor*>(overlapBuffer.block.actor->userData)->lock());
				if(!rigid.expired())
				{
					rigid.lock()->switchGravity(false);
					grabbedGrabable = rigid.lock()->grabable;
					if (!grabbedGrabable.expired())
					{
						grabbedGrabable.lock()->controller = controller;
						grabbedGrabable.lock()->OnGrab();
					}
				}
			}
		}
	}
}
