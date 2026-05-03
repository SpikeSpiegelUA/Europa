#include "D3D12Surface.h"
#include "D3D12Core.h"
namespace Europa::Graphics::D3D12 {
	namespace {

		constexpr DXGI_FORMAT ToNonSRGB(DXGI_FORMAT format) {
			if (format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			return format;
		}

	} //anonymous namespace

	void D3D12Surface::CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* commandQueue, DXGI_FORMAT format) {
		assert(factory && commandQueue);
		Release();

		if (SUCCEEDED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(uint32))) && allowTearing) {
			presentFlags = DXGI_PRESENT_ALLOW_TEARING;
		}

		DXGI_SWAP_CHAIN_DESC1 description{};
		description.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		description.BufferCount = BufferCount;
		description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		description.Flags = allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		description.Format = ToNonSRGB(format);
		description.Height = window.Height();
		description.Width = window.Width();
		description.SampleDesc.Count = 1;
		description.SampleDesc.Quality = 0;
		description.Scaling = DXGI_SCALING_STRETCH;
		description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		description.Stereo = false;

		IDXGISwapChain1* swapChain;
		HWND hwnd{ (HWND)window.Handle() };
		DXCall(factory->CreateSwapChainForHwnd(commandQueue, hwnd, &description, nullptr, nullptr, &swapChain));
		DXCall(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
		DXCall(swapChain->QueryInterface(IID_PPV_ARGS(&this->swapChain)));
		Core::Release(swapChain);

		currentBackBufferIndex = this->swapChain->GetCurrentBackBufferIndex();

		for (uint32 i{ 0 }; i < BufferCount; i++) {
			renderTargetData[i].RTV = Core::GetRTVHeap().Allocate();
		}

		Finalize();
	}
	void D3D12Surface::Present() const
	{
		assert(swapChain);
		DXCall(swapChain->Present(0, presentFlags));
		currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();
	}

	void D3D12Surface::Release()
	{
		for (uint32 i{ 0 }; i < BufferCount; i++) {
			RenderTargetData& data{ renderTargetData[i] };
			Core::Release(data.Resource);
			Core::GetRTVHeap().Free(data.RTV);
		}

		Core::Release(swapChain);
	}

	void D3D12Surface::Resize() 
	{

	}

	void D3D12Surface::Finalize()
	{
		//Create RTVs for back-buffers.
		for (uint32 i{ 0 }; i < BufferCount; i++) {
			RenderTargetData& data{ renderTargetData[i] };
			assert(!data.Resource);
			DXCall(swapChain->GetBuffer(i, IID_PPV_ARGS(&data.Resource)));
			D3D12_RENDER_TARGET_VIEW_DESC desc{};
			desc.Format = Core::GetDefaultRenderTargetFormat();
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			Core::Device()->CreateRenderTargetView(data.Resource, &desc, data.RTV.CPU);
		}

		DXGI_SWAP_CHAIN_DESC desc{};
		DXCall(swapChain->GetDesc(&desc));
		const uint32 width{ desc.BufferDesc.Width };
		const uint32 height{ desc.BufferDesc.Height };
		assert(window.Width() == width && window.Height() == height);

		//Set viewpoar and scissor rectangle.
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.Width = (float)width;
		viewport.Height = (float)height;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;

		scissorRectangle = { 0, 0, (int32)width, (int32)height };


	}
}