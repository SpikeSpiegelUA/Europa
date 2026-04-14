#pragma once
#include "D3D12CommonHeaders.h"

namespace Europa::Graphics::D3D12 {
	class DescriptorHeap;
}

namespace Europa::Graphics::D3D12::Core {
	bool Initialize();
	void Shutdown();
	void Render();

	template<typename T>
	constexpr void Release(T*& resource) {
		if (resource) {
			resource->Release();
			resource = nullptr;
		}
	}

	namespace Details {
		void DeferredRelease(IUnknown* resource);
	}

	template<typename T>
	constexpr void DeferredRelease(T*& resource) {
		if (resource) {
			Details::DeferredRelease(resource);
			resource = nullptr;
		}
	}

	ID3D12Device* const Device();
	DescriptorHeap& GetRTVHeap();
	DescriptorHeap& GetDSVHeap();
	DescriptorHeap& GetSRVHeap();
	DescriptorHeap& GetUAVHeap();
	uint32 CurrentFrameIndex();
	DXGI_FORMAT GetDefaultRenderTargetFormat();
	void SetDeferredReleasesFlag();

	Surface CreateSurface(Platform::Window window);
	void RemoveSurface(SurfaceID id);
	void ResizeSurface(SurfaceID id, uint32, uint32);
	uint32 SurfaceWidth(SurfaceID id);
	uint32 SurfaceHeight(SurfaceID id);
	void RenderSurface(SurfaceID id);
}
