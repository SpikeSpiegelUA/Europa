#pragma once
#include "D3D12CommonHeaders.h"
#include "D3D12Resources.h"

namespace Europa::Graphics::D3D12 {
	class D3D12Surface {
	public:
		explicit D3D12Surface(Platform::Window window) : window{ window } {
			assert(this->window.Handle());
		}
#if USE_STL_VECTOR
		DISABLE_COPY(D3D12Surface);
		constexpr D3D12Surface(D3D12Surface&& surface) : swapChain{ surface.swapChain },
			window{ surface.window }, currentBackBufferIndex{ surface.currentBackBufferIndex },
			viewport{ surface.viewport }, scissorRectangle{ surface.scissorRectangle } 
		{
			for (uint32 i{ 0 }; i < FrameBufferCount; i++) {
				renderTargetData[i].Resource = surface.renderTargetData[i].Resource;
				renderTargetData[i].RTV = surface.renderTargetData[i].RTV;
			}
			surface.Reset();
		}

		constexpr D3D12Surface& operator=(D3D12Surface&& surface) 
		{
			assert(this != &surface);
			if (this != &surface) {
				Release();
				Move(surface);
			}
			return *this;
		}
#endif
		~D3D12Surface() {
			Release();
		}

		void CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* commandQueue, DXGI_FORMAT format);
		void Present() const;
		void Resize();

		constexpr  uint32 Width() const 
		{ 
			return (uint32)viewport.Width; 
		}
		constexpr  uint32 Height() const 
		{
			return (uint32)viewport.Height;
		}
		constexpr ID3D12Resource* const BackBuffer() const 
		{
			return renderTargetData[currentBackBufferIndex].Resource;
		}
		constexpr D3D12_CPU_DESCRIPTOR_HANDLE RTV() const 
		{
			return renderTargetData[currentBackBufferIndex].RTV.CPU;
		}
		constexpr const D3D12_VIEWPORT& Viewport() const
		{
			return viewport;
		}
		constexpr const D3D12_RECT& ScissorRect() const
		{
			return scissorRectangle;
		}

	private:
		void Release();
		void Finalize();

#if USE_STL_VECTOR
		constexpr void Move(D3D12Surface& surface) 
		{
			swapChain = surface.swapChain;
			for (uint32 i{ 0 }; i < FrameBufferCount; i++)
				renderTargetData[i] = surface.renderTargetData[i];
			window = surface.window;
			currentBackBufferIndex = surface.currentBackBufferIndex;
			viewport = surface.viewport;
			scissorRectangle = surface.scissorRectangle;

			surface.Reset();
		}

		constexpr void Reset() 
		{
			swapChain = nullptr ;
			for (uint32 i{ 0 }; i < FrameBufferCount; i++) 
				renderTargetData[i] = {};
			window = {};
			currentBackBufferIndex = 0;
			viewport = {};
			scissorRectangle = {};
		}
#endif

		struct RenderTargetData {
			ID3D12Resource* Resource{ nullptr };
			DescriptorHandle RTV;
		};

		IDXGISwapChain4* swapChain{ nullptr };
		RenderTargetData renderTargetData[FrameBufferCount]{};
		Platform::Window window{};
		mutable uint32 currentBackBufferIndex{};
		D3D12_VIEWPORT viewport{};
		D3D12_RECT scissorRectangle{};
	};
}
