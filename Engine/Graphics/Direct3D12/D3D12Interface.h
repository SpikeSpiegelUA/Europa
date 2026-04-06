#pragma once
namespace Europa::Graphics {
	struct PlatformInterface;

	namespace D3D12 {
		void GetPlatformInterface(PlatformInterface& pi);
	}
}
