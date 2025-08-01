#include "../Content/ContentLoader.h"
#include "..\Components\Script.h"
#include "..\Platform\PlatformTypes.h"
#include "..\Platform\Platform.h"
#include "..\Graphics\Renderer.h"
#include <thread>
#if !defined(SHIPPING)

using namespace Europa;
namespace {
	Europa::Graphics::RenderSurface GameWindow{};

	LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		switch (msg) {
		case WM_DESTROY:
		{
			if (GameWindow.Window.IsClosed()) {
				PostQuitMessage(0);
				return 0;
			}
			break;
		}
		case WM_SYSCHAR:
			if (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN)) {
				GameWindow.Window.SetFullscreen(!GameWindow.Window.IsFullscreen());
				return 0;
			}
			break;
		}

		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
}

extern bool EngineInitialize() {
	if (!Content::LoadGame())
		return false;
	Platform::WindowInitInfo info{
		&WinProc, nullptr, L"Europa Game"
	};

	GameWindow.Window = Platform::EngineCreateWindow(&info);

	if (!GameWindow.Window.IsValid())
		return false;

	return true;
}
extern void EngineUpdate() {
	Europa::Script::Update(10.f);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
extern void EngineShutdown() {
	Platform::RemoveWindow(GameWindow.Window.GetID());
	Europa::Content::UnloadGame();
}
#endif