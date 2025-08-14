#pragma once
#include "../Common/CommonHeaders.h"

#ifdef _WIN64
#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif
#include <Windows.h>

namespace Europa::Platform {
	using WindowProc = LRESULT(*)(HWND, UINT, WPARAM, LPARAM);
	using WindowHandle = HWND;

	struct WindowInitInfo {
		WindowProc Callback{ nullptr };
		WindowHandle Parent{ nullptr };
		const wchar_t* Caption{ nullptr };

		int32 Left{ 0 };
		int32 Top{ 0 };
		int32 Width{ 1920 };
		int32 Height{ 1000 };
	};
}
#endif