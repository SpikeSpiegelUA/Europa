#include "Platform.h"
#include "PlatformTypes.h"

namespace Europa::Platform {
#ifdef _WIN64
	namespace {

		struct WindowInfo {
			HWND HWND{ nullptr };
			RECT ClientArea{ 0, 0, 1920, 1080 };
			RECT FullscreenArea{};
			POINT TopLeft{ 0, 0 };
			DWORD Style{ WS_VISIBLE };
			bool IsFullscreen{ false };
			bool IsClosed{ false };
		};

		Utilities::Vector<WindowInfo> Windows;
		Utilities::Vector<uint32> AvailableSlots;

		uint32 AddToWindows(WindowInfo info) {
			uint32 id{ uint32_invalid_id };
			if (AvailableSlots.empty()) {
				id = (uint32)Windows.size();
				Windows.emplace_back(info);
			}
			else {
				id = AvailableSlots.back();
				AvailableSlots.pop_back();
				assert(id != uint32_invalid_id);
				Windows[id] = info;
			}
			return id;
		}

		void RemoveFromWindows(uint32 id) {
			assert(id < Windows.size());
			AvailableSlots.emplace_back(id);
		}

		WindowInfo& GetWindowInfoFromID(WindowID id) {
			assert(id < Windows.size());
			assert(Windows[id].HWND);
			return Windows[id];
		}

		WindowInfo& GetWindowInfoFromHandle(WindowHandle handle) {
			const WindowID id{ (ID::IDType)GetWindowLongPtr(handle, GWLP_USERDATA) };
			return GetWindowInfoFromID(id);
		}

		LRESULT CALLBACK InternalWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
			WindowInfo* info{ nullptr };
			switch (msg) {
				case WM_DESTROY:
					GetWindowInfoFromHandle(hwnd).IsClosed = true;
					break;
				case WM_EXITSIZEMOVE:
					info = &GetWindowInfoFromHandle(hwnd);
					break;
				case WM_SIZE:
					if (wparam == SIZE_MAXIMIZED) {
						info = &GetWindowInfoFromHandle(hwnd);
					}
					break;
				case WM_SYSCOMMAND:
					if (wparam == SC_RESTORE) {
						info = &GetWindowInfoFromHandle(hwnd);
					}
					break;
				default:
					break;
			}

			if (info) {
				assert(info->HWND);
				GetClientRect(info->HWND, info->IsFullscreen ? &info->FullscreenArea : &info->ClientArea);
			}

			LONG_PTR longPtr{ GetWindowLongPtr(hwnd, 0) };
			return longPtr ? ((WindowProc)longPtr)(hwnd, msg, wparam, lparam) : DefWindowProc(hwnd, msg, wparam, lparam);
		}

		void ResizeWindow(const WindowInfo& windowInfo, const RECT& area) {
			//Adjust the window size for correct device size.
			RECT windowRect{ area };
			AdjustWindowRect(&windowRect, windowInfo.Style, FALSE);

			const int32 width{ windowRect.right - windowRect.left };
			const int32 height{ windowRect.bottom - windowRect.top };

			MoveWindow(windowInfo.HWND, windowInfo.TopLeft.x, windowInfo.TopLeft.y, width, height, true);
		}

		void ResizeWindow(const WindowID windowID, const int32 width, const int32 height) {
			WindowInfo& windowInfo{ GetWindowInfoFromID(windowID) };

			//NOTE: We also support a case, when the user changes the resolution in the fullscreen mode.
			RECT& area{ windowInfo.IsFullscreen ? windowInfo.FullscreenArea : windowInfo.ClientArea };
			area.bottom = area.top + height;
			area.right = area.left + width;

			ResizeWindow(windowInfo, area);
		}

		bool IsWindowFullscreen(WindowID id) {
			return GetWindowInfoFromID(id).IsFullscreen;
		}

		WindowHandle GetWindowHandle(WindowID id) {
			return GetWindowInfoFromID(id).HWND;
		}

		void SetWindowCaption(WindowID id, const wchar_t* caption) {
			WindowInfo& windowInfo{ GetWindowInfoFromID(id) };
			SetWindowText(windowInfo.HWND, caption);
		}

		const Math::UInt32Vector4 GetWindowSize(WindowID id) {
			WindowInfo& windowInfo{ GetWindowInfoFromID(id) };
			RECT area{ windowInfo.IsFullscreen ? windowInfo.FullscreenArea : windowInfo.ClientArea };
			return { (uint32)area.left, (uint32)area.top, (uint32)area.right, (uint32)area.bottom };
		}

		bool IsWindowClosed(WindowID id) {
			return GetWindowInfoFromID(id).IsClosed;
		}

		void SetWindowFullscreen(WindowID id, bool isFullscreen) {
			WindowInfo& windowInfo{ GetWindowInfoFromID(id) };
			if (windowInfo.IsFullscreen != isFullscreen) {
				windowInfo.IsFullscreen = isFullscreen;

				if (isFullscreen) {
					//Store the current window dimensions so they can be restored when switching out of fullscreen state.
					GetClientRect(windowInfo.HWND, &windowInfo.ClientArea);
					RECT rect;
					GetWindowRect(windowInfo.HWND, &rect);
					windowInfo.TopLeft.x = rect.left;
					windowInfo.TopLeft.y = rect.top;
					windowInfo.Style = 0;
					SetWindowLongPtr(windowInfo.HWND, GWL_STYLE, windowInfo.Style);
					ShowWindow(windowInfo.HWND, SW_MAXIMIZE);
				}
				else {
					windowInfo.Style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
					SetWindowLongPtr(windowInfo.HWND, GWL_STYLE, windowInfo.Style);
					ResizeWindow(windowInfo, windowInfo.ClientArea);
					ShowWindow(windowInfo.HWND, SW_SHOWNORMAL);
				}
			}
		}

	} //anonymous namespace

	Window EngineCreateWindow(const WindowInitInfo* const initInfo)
	{
		WindowProc callback{initInfo ? initInfo->Callback : nullptr };
		WindowHandle parent{initInfo ? initInfo->Parent : nullptr };

		//Create the window class.
		WNDCLASSEX wc;
		ZeroMemory(&wc, sizeof(wc));
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = InternalWindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = callback ? sizeof(callback) : 0;
		wc.hInstance = 0;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = CreateSolidBrush(RGB(26, 48, 76));
		wc.lpszMenuName = NULL;
		wc.lpszClassName = L"EuropaWindow";
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		//Register the window class.
		RegisterClassEx(&wc);

		//Create the window.
		WindowInfo info{};
		RECT rc{ info.ClientArea };

		//Adjust the window size according to the style of the window.
		AdjustWindowRect(&rc, info.Style, FALSE);

		const wchar_t* caption{initInfo && initInfo->Caption ? initInfo->Caption : L"Europa Game" };
		const int32 left{initInfo && initInfo->Left ? initInfo->Left : info.ClientArea.left };
		const int32 top{initInfo && initInfo->Top ? initInfo->Top : info.ClientArea.top };
		const int32 width{initInfo && initInfo->Width ? initInfo->Width : rc.right - rc.left };
		const int32 height{initInfo && initInfo->Height ? initInfo->Height : rc.bottom - rc.top};

		info.Style |= parent ? WS_CHILDWINDOW : WS_OVERLAPPEDWINDOW;


		info.HWND = CreateWindowEx(
			/* [in]           DWORD     dwExStyle,       */ 0,                   //Extended style.
			/* [in, optional] LPCSTR    lpClassName,	 */ wc.lpszClassName,	 //Window class name.
			/* [in, optional] LPCSTR    lpWindowName,	 */ caption,			 //Instance title.
			/* [in]           DWORD     dwStyle,		 */ info.Style,			 //Window style.
			/* [in]           int       X,				 */ left,				 //Initial window position.
			/* [in]           int       Y,				 */ top,				 //Initial window position.
			/* [in]           int       nWidth,			 */ width,				 //Initial window dimensions.
			/* [in]           int       nHeight,		 */ height,				 //Initial window dimensions.
			/* [in, optional] HWND      hWndParent,		 */ parent,				 //Handle to parent window.
			/* [in, optional] HMENU     hMenu,			 */ NULL,				 //Handle to menu.
			/* [in, optional] HINSTANCE hInstance,		 */ NULL,				 //Instance of this application.
			/* [in, optional] LPVOID    lpParam			 */ NULL				 //Extra creation parameters.
		);
		
		if (info.HWND) {
			SetLastError(0);
			const WindowID id{ AddToWindows(info) };
			SetWindowLongPtr(info.HWND, GWLP_USERDATA, (LONG_PTR)id);
			//Set in the "extra" bytes  the pointer to the window callback function which handles messages for the window.
			if (callback)
				SetWindowLongPtr(info.HWND, 0, (LONG_PTR)callback);
			assert(GetLastError() == 0);
			ShowWindow(info.HWND, SW_SHOWNORMAL);
			UpdateWindow(info.HWND);
			return Window{ id };
		}

		return {};
	}
	void RemoveWindow(WindowID id)
	{
		WindowInfo& info{ GetWindowInfoFromID(id) };
		DestroyWindow(info.HWND);
		RemoveFromWindows(id);
	}
#elif
#error "One has to implement at least one platform."
#endif

	void Window::SetFullscreen(bool isFullscreen) const {
		assert(IsValid());
		SetWindowFullscreen(ID, isFullscreen);
	}

	bool Window::IsFullscreen() const {
		assert(IsValid());
		return IsWindowFullscreen(ID);
	}

	void* Window::Handle() const {
		assert(IsValid());
		return GetWindowHandle(ID);
	}

	void Window::SetCaption(const wchar_t* caption) const {
		assert(IsValid());
		SetWindowCaption(ID, caption);
	}

	const Math::UInt32Vector4 Window::Size() const {
		assert(IsValid());
		return GetWindowSize(ID);
	}

	void Window::Resize(uint32 width, uint32 height) const {
		assert(IsValid());
		ResizeWindow(ID, width, height);
	}

	const uint32 Window::Width() const {
		Math::UInt32Vector4 s{ Size() };
		return s.z - s.x;
	}

	const uint32 Window::Height() const {
		Math::UInt32Vector4 s{ Size() };
		return s.w - s.y;
	}

	bool Window::IsClosed() const {
		assert(IsValid());
		return IsWindowClosed(ID);
	}

}