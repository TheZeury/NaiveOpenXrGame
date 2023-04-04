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
			eXrPointer = (1ui32) < 2,
			eXrPointable = (1ui32) < 3,
			eXrGrabber = (1ui32) < 4,
			eXrGrabbable = (1ui32) < 5,
		};
	};

	namespace NaiveGameSimulationFilters
	{
		const PxFilterData CommonInWorld = PxFilterData(SimulationFilterMaskBits::eInWorld, 0, 0, 0);
		const PxFilterData CommonUIHovering = PxFilterData(SimulationFilterMaskBits::eUIHovering, UINT32_MAX, 0, 0);	// Filter out everything.
		const PxFilterData XrPointer = PxFilterData(SimulationFilterMaskBits::eXrPointer, 0, SimulationFilterMaskBits::eXrPointable, 0);
		const PxFilterData XrGrabber = PxFilterData(SimulationFilterMaskBits::eXrGrabber, 0, SimulationFilterMaskBits::eXrGrabbable, 0);
		const PxFilterData XrUIPointable = PxFilterData(SimulationFilterMaskBits::eXrPointable | SimulationFilterMaskBits::eUIHovering, 0, SimulationFilterMaskBits::eXrPointer, 0);
	};
}

