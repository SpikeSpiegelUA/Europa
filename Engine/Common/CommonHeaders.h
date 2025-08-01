#pragma once
#pragma warning(disable:4530) // disable exception warning for std exceptions. Annoying!!!

// C/C++ Headers.
#include <stdint.h>
#include <assert.h>
#include <type_traits>
#include <unordered_map>
#include <memory>
#include <string>

#if defined (_WIN64)
#include "DirectXMath.h"
#endif
//Common headers.
#include "PrimitiveTypes.h"
#include "Id.h"
#include "..\Utilities\Utilities.h"
#include "..\Utilities\MathTypes.h"

//Define Macro that will remove code from Non-Debug builds.
#ifdef _DEBUG
#define DEBUG_OP(x) x
#else
#define DEBUG_OP(x) (void(0));
#endif
