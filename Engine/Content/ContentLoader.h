#pragma once
#include "../Common/CommonHeaders.h"

#if !defined(SHIPPING)
namespace Europa::Content {
	bool LoadGame();
	void UnloadGame();


	bool LoadEngineShaders(std::unique_ptr<uint8[]>& shaders, uint64& size);

}
#endif
