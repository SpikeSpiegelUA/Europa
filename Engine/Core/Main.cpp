
/*Things to do to create a game project:
	1)Generate an MSVC solution/project;
	2)Add files that contain script;
	3)Set include and library directories;
	4)Set foce include files(GameEntityAPI.h);
	5)Set C++ version and calling convension.
*/
#include "CommonHeaders.h"
#include <filesystem>

#ifdef _WIN64

#ifndef WIN32_LEAN_AND_MEAN

#define WIN32_LEAN_AND_MEAN

#endif
#include <Windows.h>
#include <crtdbg.h>
//TODO: we might want to have an IO utility header/library and move this function in there.
namespace {
	std::filesystem::path SetCurrentDirectoryToExecutablePath()
	{
		//Set the working directory to the executable path
		wchar_t path[MAX_PATH]{};
		const uint32 length{ GetModuleFileName(0, &path[0], MAX_PATH) };
		if (!length || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			return {};
		std::filesystem::path p{ path };
		std::filesystem::current_path(p.parent_path());
		return std::filesystem::current_path();
	}
}
#ifndef USE_WITH_EDITOR

extern bool EngineInitialize();
extern void EngineUpdate();
extern void EngineShutdown();

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif
	
	SetCurrentDirectoryToExecutablePath();

	if (EngineInitialize()) {
		MSG msg{};
		bool isRunning{ true };
		while (isRunning) {
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				isRunning &= (msg.message != WM_QUIT);
			}
			EngineUpdate();
		}
	}
	EngineShutdown();
	return 0;
}

#endif

#endif