#pragma once

#include "mainCommon.h"
#include "GameObject.h"
#include <unordered_set>

namespace Noxg
{
	MAKE_HANDLE(Scene);

	class Scene
	{
	public:
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

	public:
		std::weak_ptr<Scene> self;

	public:
		PxScene* physicsScene = nullptr;
		friend class SceneManager;
		friend class PhysicsEngineInstance;
		friend class PhysXInstance;
	};
}

