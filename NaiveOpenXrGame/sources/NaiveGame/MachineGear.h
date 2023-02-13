#pragma once

#include "mainCommon.h"
#include "Bricks/GameComponent.h"
#include "Renderer/Texture.h"

namespace Noxg
{
	MAKE_HANDLE(MachineGear);

	class MachineGear : public GameComponent
	{
	public:
		MachineGear(rf::Texture _texture, int _level = 1, float _baseSize = 0.1f, float _thickness = 0.1f);

		virtual void Enable() override;
		virtual void Redraw();
		virtual std::vector<PxShape*> getRecommendedColliders();
		void setLevel(int newLevel);
		void setBaseSize(float newBaseSize);

	public:
		rf::Texture texture;
		int level = 1;
		float baseSize = 0.1f;
		float thickness = 0.1f;
		const int baseAmount = 2;
	};
}

