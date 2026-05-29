#pragma once

#include "D3D12CommonHeaders.h"

namespace Europa::Graphics::D3D12 {
	struct D3D12FrameInfo;
}

namespace Europa::Graphics::D3D12::GPass 
{
	bool Initialize();
	void Shutdown();

	//NOTE:: call this every frame before rendering anything to GPass.
	void SetSize(Math::UInt32Vector2 size);
	void DepthPrepass(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& info);
	void Render(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& info);

	[[nodiscard]] const D3D12RenderTexture& MainBuffer();
	[[nodiscard]] const D3D12DepthBuffer& DepthBuffer();

	void AddTransitionsForDepthPrepass(D3DX::D3D12ResourceBarrier& barriers);
	void AddTransitionsForGPass(D3DX::D3D12ResourceBarrier& barriers);
	void AddTransitionsForPostProcess(D3DX::D3D12ResourceBarrier& barriers);

	void SetRenderTargetsForDepthPrepass(ID3D12GraphicsCommandList* cmdList);
	void SetRenderTargetsForGPass(ID3D12GraphicsCommandList* cmdList);
}