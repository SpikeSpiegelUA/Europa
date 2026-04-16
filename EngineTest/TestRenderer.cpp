
#include "TestRenderer.h"
#include "..\Platform\Platform.h"
#include "..\Platform\PlatformTypes.h"
#include "..\Graphics\Renderer.h"
#if TEST_RENDERER 

using namespace Europa;

Platform::Window Windows[4];

Graphics::RenderSurface Surfaces[4];
Timer timer{};

void DestroyRenderSurface(Graphics::RenderSurface& surface);

LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) 
	{
		case WM_DESTROY:
		{
			bool allClosed{ true };
			for (uint32 i = 0; i < _countof(Surfaces); ++i) {
				if (Surfaces[i].Window.IsValid()) 
				{
					if (Surfaces[i].Window.IsClosed()) {
						DestroyRenderSurface(Surfaces[i]);
					}
					else {
						allClosed = false;
					}
				}
			}
			if (allClosed) {
				PostQuitMessage(0);
				return 0;
			}
		}
		break;
		case WM_SYSCHAR:
			if (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN)) {
				Platform::Window win{ Platform::WindowID{(ID::IDType)GetWindowLongPtr(hwnd, GWLP_USERDATA)} };
				win.SetFullscreen(!win.IsFullscreen());
				return 0;
			}
			break;
		case WM_KEYDOWN:
			if (wparam == VK_ESCAPE) {
				PostMessage(hwnd, WM_CLOSE, 0, 0);
				return 0;
			}
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void DestroyRenderSurface(Graphics::RenderSurface& surface) {
	Graphics::RenderSurface temp{ surface };
	surface = {};
	if (temp.Surface.IsValid())
		Graphics::RemoveSurface(temp.Surface.GetID());
	if (temp.Window.IsValid())
		Platform::RemoveWindow(temp.Window.GetID());
}

void CreateRenderSurface(Graphics::RenderSurface& surface, Platform::WindowInitInfo info) {
	surface.Window = Platform::EngineCreateWindow(&info);
	surface.Surface = Graphics::CreateSurface(surface.Window);
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

	for (uint32 i{ 0 }; i < _countof(Surfaces); ++i)
		CreateRenderSurface(Surfaces[i], info[i]);
	return result;
}

void EngineTest::Run() {
	timer.Begin();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	for (uint32 i{ 0 }; i < _countof(Surfaces); i++) 
	{
		if (Surfaces[i].Surface.IsValid())
			Surfaces[i].Surface.Render();
	}
	timer.End();
}

void EngineTest::Shutdown() {
	for (uint32 i{ 0 }; i < _countof(Surfaces); ++i) {
		DestroyRenderSurface(Surfaces[i]);
	}

	Graphics::Shutdown();
}

#endif