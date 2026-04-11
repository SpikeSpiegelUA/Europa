#pragma once
#include "D3D12CommonHeaders.h"

namespace Europa::Graphics::D3D12 {
	class D3D12Surface {
	public:
		explicit D3D12Surface(Platform::Window window) : window{ window } {
			assert(this->window.Handle());
		}

		~D3D12Surface() {
			Release();
		}

		void CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* commandQueue, DXGI_FORMAT format);
		void Present() const;
		void Resize();

		uint32 Width() const {}
		uint32 Height() const {}

	private:
		void Release();

		IDXGISwapChain4* swapChain{ nullptr };
		Platform::Window window{};
		uint32 currentBackBufferIndex{};
	};
}
