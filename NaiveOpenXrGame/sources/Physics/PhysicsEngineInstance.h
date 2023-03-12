#pragma once

#include "mainCommon.h"
#include "Bricks/Scene.h"
#include <list>

namespace Noxg
{
	MAKE_HANDLE(PhysicsEngineInstance);

	class PhysicsEngineInstance
	{
	public:
		virtual void Initialize() = 0;
		virtual void Simulate(float timeDelta) = 0;
		virtual void addScene(rf::Scene scene) = 0;
		virtual PxScene* createScene() const = 0;
		virtual PxRigidDynamic* createRigidDynamic(const PxTransform& pose) const = 0;
		virtual PxRigidStatic* createRigidStatic(const PxTransform& pose) const = 0;
		virtual PxShape* createShape(const PxGeometry& geometry, const PxFilterData& simulationFilterData) const = 0;
	protected:
		std::list<rf::Scene> scenes;
	};

	struct SimulationFilterMaskBits
	{
		enum : uint32_t
		{
			eInWorld = 1ui32,
			eUIHovering = (1ui32) < 1,
		};
	};

	namespace NaiveGameSimulationFilters
	{
		const PxFilterData CommonInWorld = PxFilterData(SimulationFilterMaskBits::eInWorld, 0, 0, 0);
		const PxFilterData CommonUIHovering = PxFilterData(SimulationFilterMaskBits::eUIHovering, UINT32_MAX, 0, 0);	// Filter out everything.
	};
}

