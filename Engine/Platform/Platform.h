#pragma once
#include "../Common/CommonHeaders.h"
#include "window.h"

namespace Europa::Platform {

	struct WindowInitInfo;

	Window EngineCreateWindow(const WindowInitInfo* const initInfo = nullptr);
	void RemoveWindow(WindowID id);

}