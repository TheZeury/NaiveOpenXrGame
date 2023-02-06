#pragma once

#include "mainCommon.h"
#include "GameComponent.h"

namespace Noxg
{
	MAKE_HANDLE(ITransform);

	class ITransform : public GameComponent
	{
	public:
		virtual std::tuple<bool, glm::mat4*> updateMatrix() = 0;

		virtual glm::vec3 getGlobalPosition() = 0;
		virtual void setGlobalPosition(glm::vec3 pos) = 0;

		virtual glm::quat getGlobalRotation() = 0;
		virtual void setGlobalRotation(glm::quat rotat) = 0;
		
		virtual glm::vec3 getGlobalScale() = 0;
		virtual void setGlobalScale(glm::vec3 scal) = 0;

		virtual glm::mat4 getGlobalMatrix() = 0;
		virtual void setGlobalMatrix(const glm::mat4& mat) = 0;

		virtual glm::vec3 getLocalPosition() = 0;
		virtual void setLocalPosition(glm::vec3 pos) = 0;

		virtual glm::quat getLocalRotation() = 0;
		virtual void setLocalRotation(glm::quat rotat) = 0;

		virtual glm::vec3 getLocalScale() = 0;
		virtual void setLocalScale(glm::vec3 scal) = 0;

		virtual glm::mat4 getLocalMatrix() = 0;
		virtual void setLocalMatrix(const glm::mat4& mat) = 0;

		virtual void Enable() override = 0;

		virtual void addChild(hd::ITransform child)
		{
			if (child != nullptr)
			{
				child->parent = std::dynamic_pointer_cast<ITransform>(shared_from_this());
				children.push_back(child);
			}
		}

		rf::ITransform parent;
		std::vector<rf::ITransform> children;

		bool globalChanged = true;
		bool localChanged = true;
	};

	MAKE_HANDLE(Transform);

	class Transform : public ITransform
	{
	public:
		virtual std::tuple<bool, glm::mat4*> updateMatrix() override;

		virtual glm::vec3 getGlobalPosition() override { return { }; }	// TODO
		virtual void setGlobalPosition(glm::vec3 pos) override { }	// TODO

		virtual glm::quat getGlobalRotation() override { return { }; }	// TODO
		virtual void setGlobalRotation(glm::quat rotat) override { }	// TODO

		virtual glm::vec3 getGlobalScale() override { return { }; }	// TODO
		virtual void setGlobalScale(glm::vec3 scal) override { }	// TODO

		virtual glm::mat4 getGlobalMatrix() override;
		virtual void setGlobalMatrix(const glm::mat4& mat) override;

		virtual glm::vec3 getLocalPosition() override;
		virtual void setLocalPosition(glm::vec3 pos) override;

		virtual glm::quat getLocalRotation() override;
		virtual void setLocalRotation(glm::quat rotat) override;

		virtual glm::vec3 getLocalScale() override;
		virtual void setLocalScale(glm::vec3 scal) override;

		virtual glm::mat4 getLocalMatrix() override;
		virtual void setLocalMatrix(const glm::mat4& mat) override;

		virtual void Enable() override { }	// TODO

	protected:
		glm::mat4 globalMatrix{ 1.f };

		glm::vec3 localPosition = { 0.f, 0.f, 0.f };
		glm::quat localRotation = { 1.f, 0.f, 0.f, 0.f };
		glm::vec3 localScale = { 1.f, 1.f, 1.f };
		glm::mat4 localMatrix{ 1.f };
	};
}

