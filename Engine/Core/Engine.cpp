#include "../Content/ContentLoader.h"
#include "..\Components\Script.h"
#include <thread>
#if !defined(SHIPPING)
extern bool EngineInitialize() {
	bool result {Europa::Content::LoadGame()};
	return result;
}
extern void EngineUpdate() {
	Europa::Script::Update(10.f);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
extern void EngineShutdown() {
	Europa::Content::UnloadGame();
}
#endif