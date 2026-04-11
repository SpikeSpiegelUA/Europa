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

		DXGI_SWAP_CHAIN_DESC1 description{};
		description.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		description.BufferCount = FrameBufferCount;
		description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		description.Flags = 0;
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
	}
}