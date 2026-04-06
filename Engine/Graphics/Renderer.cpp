#include "Renderer.h"
#include "GraphicsPlatformInterface.h"
#include "Direct3D12/D3D12Interface.h"

namespace Europa::Graphics {
	namespace {
		PlatformInterface GFX{};
		bool SetPlatformInterface(GraphicsPlatform platform) {
			switch (platform) {
				case GraphicsPlatform::Direct3D12:
					D3D12::GetPlatformInterface(GFX);
					break;
				default:
					return false;
			}
			return true;
		}
	}

	bool Initialize(GraphicsPlatform platform) {
		return SetPlatformInterface(platform) && GFX.Initialize();
	}

	void Shutdown() {
		GFX.Shutdown();
	}

	void Renderer() {
		GFX.Render();
	}
}