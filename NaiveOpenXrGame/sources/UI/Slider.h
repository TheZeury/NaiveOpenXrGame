#pragma once

#include "mainCommon.h"

namespace Noxg
{
	template<typename T> class Slider;
	namespace hd {
		template<typename T>
		using Slider = std::shared_ptr<Noxg::Slider<T>>;
	}
	namespace rf {
		template<typename T>
		using Slider = std::weak_ptr<Noxg::Slider<T>>;
	}

	template<typename T>
	class Slider
	{
	public:
		float rawValue; // 0.0f to 1.0f

		auto virtual getValue() const -> T	// Real value not necessary to be between 0.0f and 1.0f.
		{
			return getRaw();
		}

		auto virtual setValue(T value) -> void
		{
			setRaw(value);
		}

		auto getRaw() const -> float
		{
			return rawValue;
		}

		auto setRaw(float value) -> void
		{
			rawValue = value;
			updatePosition();
		}

	private:
		auto virtual updatePosition() -> void
		{
			// TODO
		}
	};
}

