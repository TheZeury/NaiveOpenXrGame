#pragma once

#include "mainCommon.h"
#include "GameObject.h"

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
		std::unordered_set<hd::GameObject> gameObjects;
		rf::ITransform cameraTransform;
		hd::Scene debugScene;
		bool onlyDebug = false;

	public:
		std::weak_ptr<SceneManager> manager;

	public:
		PxScene* physicsScene = nullptr;
		friend class SceneManager;
		friend class PhysicsEngineInstance;
		friend class PhysXInstance;
	};
}

