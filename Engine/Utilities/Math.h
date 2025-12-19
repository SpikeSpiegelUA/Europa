#pragma once
#include "../Common/CommonHeaders.h"

namespace Europa::Math {
	template<typename T>
	constexpr T Clamp(T value, T min, T max) 
	{
		return (value < min) ? min : (value > max) ? max : value;
	}

	template<uint32 bits>
	constexpr uint32 PackUnitFloat(float32 f) {
		static_assert(bits <= sizeof(uint32) * 8);
		assert(f >= 0.f && f <= 1.f);
		constexpr float32 intervals{ (float32)((uint32)(1) << bits) - 1 };
		return (uint32)(intervals * f + 0.5f);
	}

	template<uint32 bits>
	constexpr float32 UnpackToUnitFloat(uint32 i) {
		static_assert(bits <= sizeof(uint32) * 8);
		assert(i < ((uint32)(1) << bits));
		constexpr float32 intervals{ (float32)(((uint32)(1) << bits) - 1) };
		return (float32)i / intervals;
	}

	template<uint32 bits>
	constexpr uint32 PackFloat(float32 f, float32 min, float32 max) {
		assert(min < max);
		assert(f <= max && f >= min);
		const float32 distance{ (f - min) / (max - min) };
		return PackUnitFloat<bits>(distance);
	}

	template<uint32 bits>
	constexpr float32 UnpackToFloat(uint32 i, float32 min, float32 max) {
		assert(min < max);
		return UnpackToUnitFloat<bits>(i) * (max - min) + min;
	}
}