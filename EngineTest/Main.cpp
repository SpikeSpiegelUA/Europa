#pragma comment(lib, "engine.lib")

#define TEST_ENTITY_COMPONENTS 0
#define TEST_WINDOW 1

#if TEST_ENTITY_COMPONENTS
#include "TestEntityComponents.h"
#elif TEST_WINDOW
#include "TestWindow.h"
#else
#error One of the tests needs to be enabled.
#endif

#ifdef _WIN64
#include <Windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	EngineTest engineTest;
	if (engineTest.Initialize()) {
		MSG msg{};
		bool isRunning{ true };
		while (isRunning) {
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				isRunning &= (msg.message != WM_QUIT);
			}
			engineTest.Run();
		}
	}
	engineTest.Shutdown();
	return 0;
}
#else
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
#endif