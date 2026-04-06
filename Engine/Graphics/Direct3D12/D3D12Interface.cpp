#include "CommonHeaders.h"
#include "D3D12Interface.h"
#include "D3D12Core.h"
#include "Graphics\GraphicsPlatformInterface.h"

namespace Europa::Graphics::D3D12 {
	void GetPlatformInterface(PlatformInterface& pi) {
		pi.Initialize = Core::Initialize;
		pi.Shutdown = Core::Shutdown;
		pi.Render = Core::Render;
	}
}