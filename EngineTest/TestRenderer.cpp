
#include "TestRenderer.h"
#include "..\Platform\Platform.h"
#include "..\Platform\PlatformTypes.h"
#include "..\Graphics\Renderer.h"
#if TEST_RENDERER 

using namespace Europa;

Platform::Window Windows[4];

Graphics::RenderSurface Surfaces[4];

LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_DESTROY:
	{
		bool allClosed{ true };
		for (uint32 i = 0; i < _countof(Surfaces); i++) {
			if (!Surfaces[i].Window.IsClosed()) {
				allClosed = false;
				break;
			}
		}
		if (allClosed) {
			PostQuitMessage(0);
			return 0;
		}
		break;
	}
	case WM_SYSCHAR:
		if (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN)) {
			Platform::Window win{ Platform::WindowID{(ID::IDType)GetWindowLongPtr(hwnd, GWLP_USERDATA)} };
			win.SetFullscreen(!win.IsFullscreen());
			return 0;
		}
		break;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void CreateRenderSurface(Graphics::RenderSurface& surface, Platform::WindowInitInfo info) {
	surface.Window = Platform::EngineCreateWindow(&info);
}

void DestroyRenderSurface(Graphics::RenderSurface& surface) {
	Platform::RemoveWindow(surface.Window.GetID());
}

bool EngineTest::Initialize() {
	bool result{ Graphics::Initialize(Graphics::GraphicsPlatform::Direct3D12) };
	if (!result)
		return result;
	Platform::WindowInitInfo info[]{
		{&WinProc, nullptr, L"Render Window 1", 100, 100, 400, 800},
		{&WinProc, nullptr, L"Render Window 2", 150, 150, 800, 400},
		{&WinProc, nullptr, L"Render Window 3", 200, 200, 400, 400},
		{&WinProc, nullptr, L"Render Window 4", 250, 250, 800, 600},
	};
	static_assert(_countof(info) == _countof(Surfaces));

	for (uint32 i{ 0 }; i < _countof(Windows); ++i)
		CreateRenderSurface(Surfaces[i], info[i]);
	return result;
}

void EngineTest::Run() {
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	Graphics::Render();
}

void EngineTest::Shutdown() {
	for (uint32 i{ 0 }; i < _countof(Surfaces); ++i) {
		DestroyRenderSurface(Surfaces[i]);
	}

	Graphics::Shutdown();
}

#endif