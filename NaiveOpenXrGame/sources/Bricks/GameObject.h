#pragma once

#include "mainCommon.h"
#include "GameComponent.h"
#include "Transform.h"
#include "Renderer/MeshModel.h"
#include "Renderer/TextModel.h"
#include "Renderer/UIElement.h"

namespace Noxg
{
	class Scene;

	MAKE_HANDLE(GameObject);

	class GameObject : public std::enable_shared_from_this<GameObject>
	{
	public:
		GameObject();

		void addComponent(hd::GameComponent component);

		std::weak_ptr<Scene> scene;

		hd::ITransform transform;

		auto addModel(hd::MeshModel model) -> void;
		auto addText(hd::TextModel text) -> void;
		auto addUIElement(hd::UIElement element) -> void;

		auto removeModel(hd::MeshModel model) -> void;
		auto removeText(hd::TextModel text) -> void;
		auto removeUIElement(hd::UIElement element) -> void;

		std::unordered_set<hd::MeshModel> models;
		std::unordered_set<hd::TextModel> texts;
		std::unordered_set<hd::UIElement> uiElements;

		std::vector<hd::GameComponent> components;
	private:
	};
}

