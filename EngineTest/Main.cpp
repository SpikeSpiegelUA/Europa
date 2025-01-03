#include "TestEntityComponents.h"

#pragma comment(lib, "engine.lib")

#define TEST_ENTITY_COMPONENTS 1

#if TEST_ENTITY_COMPONENTS

#else
#error One of the tests needs to be enabled.
#endif

int main() {
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	EngineTest engineTest{};
	if (engineTest.Initialize()) {
		engineTest.Run();
	}
	engineTest.Shutdown();
}