#pragma once

#include "mainCommon.h"
#include "GameObject.h"
#include "Renderer/UIElement.h"

namespace Noxg
{
	class SceneManager;

	MAKE_HANDLE(Scene);

	class Scene : public std::enable_shared_from_this<Scene>
	{
	public:
		void CalculateFrame();

		/// <summary>
		/// Add a new gameObject into this scene.
		/// </summary>
		/// <param name="obj"> the gameObject to be added </param>
		/// <seealso cref="removeGameObject"/>
		void addGameObject(hd::GameObject obj);

		/// <summary>
		/// Remove a particular gameObject.
		/// </summary>
		/// <param name="obj"> the gameObject to be removed. </param>
		/// <seealso cref="addGameObject"/>
		void removeGameObject(hd::GameObject obj);

		auto addModel(hd::MeshModel model, hd::ITransform transform) -> void;
		auto addText(hd::TextModel text, hd::ITransform transform) -> void;
		auto addUIElement(hd::UIElement element, hd::ITransform transform) -> void;

		auto removeModel(hd::MeshModel model, hd::ITransform transform) -> void;
		auto removeText(hd::TextModel text, hd::ITransform transform) -> void;
		auto removeUIElement(hd::UIElement element, hd::ITransform transform) -> void;

		std::unordered_set<hd::GameObject> gameObjects;
		rf::ITransform cameraTransform;
		hd::Scene debugScene;
		bool onlyDebug = false;

		std::set<std::pair<hd::MeshModel, hd::ITransform>> models;
		std::set<std::pair<hd::TextModel, hd::ITransform>> texts;
		std::set<std::pair<hd::MeshModel, hd::ITransform>> wireModels;
		std::set<std::pair<hd::UIElement, hd::ITransform>> uiElements;

	public:
		std::weak_ptr<SceneManager> manager;

	public:
		PxScene* physicsScene = nullptr;
		friend class SceneManager;
		friend class PhysicsEngineInstance;
		friend class PhysXInstance;
	};
}

