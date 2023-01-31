#pragma once

#include "../Transform.h"

namespace Noxg
{
	class PhysicsTransform : public ITransform
	{
	public:
		virtual glm::vec3 getPosition() override;
		virtual void setPosition(glm::vec3 pos) override;

		virtual glm::quat getRotation() override;
		virtual void setRotation(glm::quat rotat) override;

		virtual glm::vec3 getScale() override;
		virtual void setScale(glm::vec3 scal) override;

		virtual glm::mat4 getMatrix() override;
		virtual void setMatrix(const glm::mat4& mat) override;

		PhysicsTransform(PxRigidActor* actor) : pxActor{ actor } { }

	private:
		glm::vec3 scale = { 1.f, 1.f, 1.f };
		PxRigidActor* pxActor = nullptr;
	};
}
