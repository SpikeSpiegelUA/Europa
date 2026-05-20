#include "D3D12Helpers.h"
#include "D3D12Core.h"

namespace Europa::Graphics::D3D12::D3DX {
	namespace {

	} //anonymous namespace

	ID3D12RootSignature* CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC2& desc) {
		D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDescription{};
		versionedRootSignatureDescription.Version = D3D_ROOT_SIGNATURE_VERSION_1_2;
		versionedRootSignatureDescription.Desc_1_2 = desc;

		using namespace Microsoft::WRL;
		ComPtr<ID3DBlob> rootSignatureBlob{ nullptr };
		ComPtr<ID3DBlob> errorBlob{ nullptr };
		HRESULT hr{ S_OK };
		if (FAILED(hr = D3D12SerializeVersionedRootSignature(&versionedRootSignatureDescription, &rootSignatureBlob, &errorBlob)))
		{
			DEBUG_OP(const char* errorMessage{ errorBlob ? (const char*)errorBlob->GetBufferPointer() : "" });
			DEBUG_OP(OutputDebugStringA(errorMessage));
			return nullptr;
		}

		ID3D12RootSignature* signature{ nullptr };
		DXCall(hr = Core::Device()->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
			rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&signature)));

		if (FAILED(hr)) 
		{
			Core::Release(signature);
		}

		return signature;
	}

	ID3D12PipelineState* CreatePipelineState(D3D12_PIPELINE_STATE_STREAM_DESC desc)
	{
		assert(desc.pPipelineStateSubobjectStream && desc.SizeInBytes);
		ID3D12PipelineState* pso{ nullptr };
		DXCall(Core::Device()->CreatePipelineState(&desc, IID_PPV_ARGS(&pso)));
		assert(pso);
		return pso;
	}

	ID3D12PipelineState* CreatePipelineState(void* stream, uint64 streamSize) 
	{
		assert(stream && streamSize);
		D3D12_PIPELINE_STATE_STREAM_DESC desc{};
		desc.pPipelineStateSubobjectStream = stream;
		return CreatePipelineState(desc);
	}

}