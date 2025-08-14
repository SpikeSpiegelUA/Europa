#pragma once
#include "stdint.h"

//Unsigned integers.

using uint64 = uint64_t;
using uint32 = uint32_t;
using uint16 = uint16_t;
using uint8 = uint8_t;

//Signed integers.

using int64 = int64_t;
using int32 = int32_t;
using int16 = int16_t;
using int8 = int8_t;

//Float types.
using float32 = float;

constexpr uint64 uint64_invalid_id{ 0xffff'ffff'ffff'ffffui64 };
constexpr uint32 uint32_invalid_id{ 0xffff'ffffui32 };
constexpr uint16 uint16_invalid_id{ 0xffffui16 };
constexpr uint8 uint8_invalid_id{ 0xffui8 };


