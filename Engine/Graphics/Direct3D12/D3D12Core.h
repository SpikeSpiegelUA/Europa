#pragma once
#include "D3D12CommonHeaders.h"
#include "D3D12Resources.h"

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
}
