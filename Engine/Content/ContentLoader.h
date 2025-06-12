#pragma once
#include "CommonHeaders.h"

#if !defined(SHIPPING)
namespace Europa::Content {
	bool LoadGame();
	void UnloadGame();
}
#endif
