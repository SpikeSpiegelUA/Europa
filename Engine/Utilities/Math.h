#pragma once
#include "../Common/CommonHeaders.h"

namespace Europa::Math {
	template<typename T>
	constexpr T Clamp(T value, T min, T max) 
	{
		return (value < min) ? min : (value > max) ? max : value;
	}
}