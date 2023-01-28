#pragma once

#include "mainCommon.h"

namespace Noxg
{
	class ITransform
	{
	public:
		virtual glm::vec3 getPosition() = 0;
		virtual void setPosition(glm::vec3 pos) = 0;

		virtual glm::quat getRotation() = 0;
		virtual void setRotation(glm::quat rotat) = 0;
		
		virtual glm::vec3 getScale() = 0;
		virtual void setScale(glm::vec3 scal) = 0;

		virtual glm::mat4 getMatrix() = 0;
		virtual void setMatrix(const glm::mat4& mat) = 0;
	};

	class Transform : public ITransform
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

	private:
		glm::vec3 position = { 0.f, 0.f, 0.f };
		glm::quat rotation = { 1.f, 0.f, 0.f, 0.f };
		glm::vec3 scale = { 1.f, 1.f, 1.f };
		glm::mat4 matrix;
		bool changed = true;
	};
}

