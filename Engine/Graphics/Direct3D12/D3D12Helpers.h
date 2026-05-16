#pragma once
#include "D3D12CommonHeaders.h"

namespace Europa::Graphics::D3D12::D3DX {
	constexpr struct {
		D3D12_HEAP_PROPERTIES DefaultHeap
		{
			 D3D12_HEAP_TYPE_DEFAULT,		  //Type;
			 D3D12_CPU_PAGE_PROPERTY_UNKNOWN, //CPUPageProperty;
			 D3D12_MEMORY_POOL_UNKNOWN,		  //MemoryPoolPreference;	
			 0,								  //CreationNodeMask;
			 0								  //VisibleNodeMask;
		};
	} HeapProperties;

	ID3D12RootSignature* CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC2& desc);

	struct D3D12DescriptorRange : public D3D12_DESCRIPTOR_RANGE1
	{
		constexpr explicit D3D12DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, uint32 descriptorCount,
			uint32 shaderRegister, uint32 space = 0, D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE,
			uint32 offsetFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
			:D3D12_DESCRIPTOR_RANGE1{ rangeType, descriptorCount, shaderRegister, space, flags, offsetFromTableStart }
		{

		}
	};

	struct D3D12RootParameter : public D3D12_ROOT_PARAMETER1
	{
		constexpr void AsConstants(uint32 numConstants, D3D12_SHADER_VISIBILITY shaderVisibility,
			uint32 shaderRegister, uint32 space = 0)
		{
			ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			ShaderVisibility = shaderVisibility;
			Constants.Num32BitValues = numConstants;
			Constants.RegisterSpace = space;
		}

		constexpr void AsCBV(D3D12_SHADER_VISIBILITY shaderVisibility, uint32 shaderRegister,
			uint32 space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) {
			AsDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, shaderVisibility, shaderRegister, space, flags);
		}

		constexpr void AsSRV(D3D12_SHADER_VISIBILITY shaderVisibility, uint32 shaderRegister,
			uint32 space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) {
			AsDescriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, shaderVisibility, shaderRegister, space, flags);
		}

		constexpr void AsUAV(D3D12_SHADER_VISIBILITY shaderVisibility, uint32 shaderRegister,
			uint32 space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE) {
			AsDescriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, shaderVisibility, shaderRegister, space, flags);
		}

		constexpr void AsDescriptorTable(D3D12_SHADER_VISIBILITY visibility, const D3D12DescriptorRange* ranges,
			uint32 rangeCount)
		{
			ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			ShaderVisibility = visibility;
			DescriptorTable.NumDescriptorRanges = rangeCount;
			DescriptorTable.pDescriptorRanges = ranges;
		}
	private:
		constexpr void AsDescriptor(D3D12_ROOT_PARAMETER_TYPE type, D3D12_SHADER_VISIBILITY visibility,
			uint32 shaderRegister, uint32 space, D3D12_ROOT_DESCRIPTOR_FLAGS flags)
		{
			ParameterType = type;
			ShaderVisibility = visibility;
			Descriptor.ShaderRegister = shaderRegister;
			Descriptor.RegisterSpace = space;
			Descriptor.Flags = flags;
		}
	};

	//MAXIMUM 64 DWORDs (uint32's) divided up amongst all root parameters.
	//Root constants = 1 DWORD per constant
	//Root descriptor (CBV, SRV or UAV) = 2 DWORDs each
	//Descriptor table pointer = 1 DWORD
	//Static samplers = 0 DWORDs (compiled into shader)
	struct D3D12RootSignatureDescription : public D3D12_ROOT_SIGNATURE_DESC2
	{
		constexpr explicit D3D12RootSignatureDescription(const D3D12RootParameter* parameters,
			uint32 parameterCount, const D3D12_STATIC_SAMPLER_DESC1* staticSamplers = nullptr,
			uint32 samplerCount = 0, D3D12_ROOT_SIGNATURE_FLAGS flags =
			D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS) 
			: D3D12_ROOT_SIGNATURE_DESC2{parameterCount, parameters, samplerCount, staticSamplers, flags}
		{

		}

		ID3D12RootSignature* Create() const 
		{
			return CreateRootSignature(*this);
		}
	};
}