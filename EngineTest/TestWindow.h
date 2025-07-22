#pragma once
#include "Test.h"
#include "..\Platform\Platform.h"
#include "..\Platform\PlatformTypes.h"

using namespace Europa;

Platform::Window Windows[4];

LRESULT WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
		case WM_DESTROY:
		{
			bool allClosed{ true };
			for (uint32 i = 0; i < _countof(Windows); i++) {
				if (Windows[i].IsClosed()) {
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

class EngineTest : public Test {
public:
	bool Initialize() override {

		Platform::WindowInitInfo info[]{
			{&WinProc, nullptr, L"Test Window 1", 100, 100, 400, 800},
			{&WinProc, nullptr, L"Test Window 2", 150, 150, 800, 400},
			{&WinProc, nullptr, L"Test Window 3", 200, 200, 400, 400},
			{&WinProc, nullptr, L"Test Window 4", 250, 250, 800, 600},
		};
		static_assert(_countof(info) == _countof(Windows));

		for (uint32 i{ 0 }; i < _countof(Windows); ++i)
			Windows[i] = Platform::EngineCreateWindow(&info[i]);
		return true;
	}

	void Run() override {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	void Shutdown() override {
		for (uint32 i{ 0 }; i < _countof(Windows); ++i) {
			Platform::RemoveWindow(Windows[i].GetID());
		}
	}
};