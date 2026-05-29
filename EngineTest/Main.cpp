#pragma comment(lib, "engine.lib")
#include "Test.h"

#if TEST_ENTITY_COMPONENTS
#include "TestEntityComponents.h"
#elif TEST_WINDOW
#include "TestWindow.h"
#elif TEST_RENDERER
#include "TestRenderer.h"
#else
#error One of the tests needs to be enabled.
#endif

#ifdef _WIN64
#include <Windows.h>
#include <filesystem>

//TODO: duplicate!
std::filesystem::path SetCurrentDirectoryToExecutablePath()
{
	//Set the working directory to the executable path
	wchar_t path[MAX_PATH]{};
	const uint32_t length{ GetModuleFileName(0, &path[0], MAX_PATH) };
	if (!length || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		return {};
	std::filesystem::path p{ path };
	std::filesystem::current_path(p.parent_path());
	return std::filesystem::current_path();
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	SetCurrentDirectoryToExecutablePath();
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