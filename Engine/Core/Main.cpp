
/*Things to do to create a game project:
	1)Generate an MSVC solution/project;
	2)Add files that contain script;
	3)Set include and library directories;
	4)Set foce include files(GameEntityAPI.h);
	5)Set C++ version and calling convension.
*/

#ifdef _WIN64

#ifndef WIN32_LEAN_AND_MEAN

#define WIN32_LEAN_AND_MEAN

#endif
#include <Windows.h>
#include <crtdbg.h>
#ifndef USE_WITH_EDITOR

extern bool EngineInitialize();
extern void EngineUpdate();
extern void EngineShutdown();

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif
	if (EngineInitialize()) {
		MSG msg{};
		bool isRunning{ true };
		while (isRunning) {
			EngineUpdate();
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				isRunning &= (msg.message != WM_QUIT);
			}
		}
	}
	EngineShutdown();
	return 0;
}

#endif

#endif