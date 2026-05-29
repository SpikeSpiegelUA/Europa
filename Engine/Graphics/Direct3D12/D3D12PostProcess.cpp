#include "D3D12PostProcess.h"
#include "D3D12Shaders.h"
#include "D3D12Core.h"
#include "D3D12Surface.h"
#include "D3D12GPass.h"

namespace Europa::Graphics::D3D12::FX {
	namespace {

		struct FXRootParameterIndices 
		{
			enum : uint32 {
				RootConstants,
				DescriptorTable,
				Count
			};
		};

		ID3D12RootSignature* FXRootSignature{ nullptr };
		ID3D12PipelineState* FXPSO{ nullptr };

		bool CreateFXPSOAndRootSignature() 
		{
			assert(!FXRootSignature && !FXPSO);

			//Create Post-process FX root signature
			D3DX::D3D12DescriptorRange range{
				D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND, 0, 0,
				D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE
			};

			using IDX = FXRootParameterIndices;
			D3DX::D3D12RootParameter parameters[IDX::Count]{};
			parameters[IDX::RootConstants].AsConstants(1, D3D12_SHADER_VISIBILITY_PIXEL, 1);
			parameters[IDX::DescriptorTable].AsDescriptorTable(D3D12_SHADER_VISIBILITY_PIXEL, &range, 1);
			const D3DX::D3D12RootSignatureDescription rootSignature{ &parameters[0], _countof(parameters) };
			FXRootSignature = rootSignature.Create();
			assert(FXRootSignature);
			NAME_D3D12_OBJECT(FXRootSignature, L"Post-process FX Root Signature");
			//Create Post-process FX PSO

			struct {
				D3DX::D3D12PipelineStateSubobjectRootSignature rootSignature{ FXRootSignature };
				D3DX::D3D12PipelineStateSubobjectVS vs{ Shaders::GetEngineShader(Shaders::EngineShader::FullscreenTriangleVS) };
				D3DX::D3D12PipelineStateSubobjectPS ps{ Shaders::GetEngineShader(Shaders::EngineShader::PostProcessPS) };
				D3DX::D3D12PipelineStateSubobjectPrimitiveTopology primitiveTopology{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE };
				D3DX::D3D12PipelineStateSubobjectRenderTargetFormats renderTargetFormats;
				D3DX::D3D12PipelineStateSubobjectRasterizer rasterizer{ D3DX::RasterizerState.NoCulling };
			} stream;

			D3D12_RT_FORMAT_ARRAY rtfArray{};
			rtfArray.NumRenderTargets = 1;
			rtfArray.RTFormats[0] = D3D12Surface::defaultBackBufferFormat;

			stream.renderTargetFormats = rtfArray;

			FXPSO = D3DX::CreatePipelineState(&stream, sizeof(stream));
			NAME_D3D12_OBJECT(FXPSO, L"Post-process FX Pipeline State Object");

			return FXRootSignature && FXPSO;
			return true;
		}

	} //anonymous namespace

	bool Initialize() {
		return CreateFXPSOAndRootSignature();
	}

	void Shutdown() {
		Core::Release(FXRootSignature);
		Core::Release(FXPSO);
	}

	void PostProcess(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE targetRTV) 
	{
		cmdList->SetGraphicsRootSignature(FXRootSignature);
		cmdList->SetPipelineState(FXPSO);

		using IDX = FXRootParameterIndices;
		cmdList->SetGraphicsRoot32BitConstant(IDX::RootConstants, GPass::MainBuffer().SRV().index, 0);
		cmdList->SetGraphicsRootDescriptorTable(IDX::DescriptorTable, Core::GetSRVHeap().GPUStart());
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//NOTE: we don't need to clear the render target, because. each pixel will
		//be overwritte by pixels from GPass main buffer.
		//We also don't need a depth buffer.
		cmdList->OMSetRenderTargets(1, &targetRTV, 1, nullptr);
		cmdList->DrawInstanced(3, 1, 0, 0);
	}
}