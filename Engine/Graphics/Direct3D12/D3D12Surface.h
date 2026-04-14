#pragma once
#include "D3D12CommonHeaders.h"
#include "D3D12Resources.h"

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
