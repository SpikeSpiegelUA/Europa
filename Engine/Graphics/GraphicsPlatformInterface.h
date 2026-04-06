#pragma once
#include "CommonHeaders.h"
#include "Renderer.h"

namespace Europa::Graphics {
	struct PlatformInterface {
		bool(*Initialize)(void);
		void(*Shutdown)(void);
		void(*Render)(void);
	};
}