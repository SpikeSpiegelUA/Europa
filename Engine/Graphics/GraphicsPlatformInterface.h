#pragma once
#include "CommonHeaders.h"
#include "Renderer.h"

namespace Europa::Graphics {
	struct PlatformInterface {
		bool(*Initialize)(void);
		void(*Shutdown)(void);

		struct {
			Surface(*Create)(Platform::Window);
			void(*Remove)(SurfaceID);
			void(*Resize)(SurfaceID, uint32, uint32);
			uint32(*Height)(SurfaceID);
			uint32(*Width)(SurfaceID);
			void(*Render)(SurfaceID);
		} Surface;
	};
}