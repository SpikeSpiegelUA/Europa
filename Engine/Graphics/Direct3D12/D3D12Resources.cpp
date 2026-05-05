#include "D3D12Resources.h"
#include "D3D12Core.h"
#include "D3D12Helpers.h"

namespace Europa::Graphics::D3D12 {
#pragma region DescriptorHeap
	bool DescriptorHeap::Initialize(uint32 capacity, bool isShaderVisible) {
		std::lock_guard lock{ mutex };
		assert(capacity && capacity < D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2);
		assert(!(type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER && 
			capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE));
		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV || type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV) {
			isShaderVisible = false;
		}

		Release();

		auto* const device{ Core::Device() };
		assert(device);

		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE :
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NumDescriptors = capacity;
		desc.Type = type;
		desc.NodeMask = 0;
		
		HRESULT hr{ S_OK };
		DXCall(hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap)));
		if (FAILED(hr))
			return false;

		freeHandles = std::move(std::make_unique<uint32[]>(capacity));
		this->capacity = capacity;
		size = 0;

		for (uint32 i{ 0 }; i < capacity; i++)
			freeHandles[i] = i;
		DEBUG_OP(for (uint32 i{ 0 }; i < FrameBufferCount; ++i)
			assert(deferredFreeIndices[i].Empty()));

		descriptorSize = device->GetDescriptorHandleIncrementSize(type);
		cpuStart = heap->GetCPUDescriptorHandleForHeapStart();
		gpuStart = isShaderVisible ? heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };

		return true;
	}

	void DescriptorHeap::ProcessDeferredFree(uint32 frameIndex)
	{
		std::lock_guard lock{ mutex };
		assert(frameIndex < FrameBufferCount);

		Utilities::Vector<uint32>& indices{ deferredFreeIndices[frameIndex] };
		if (!indices.Empty()) {
			for (auto index : indices) {
				--size;
				freeHandles[size] = index;
			}
			indices.Clear();
		}
	}

	void DescriptorHeap::Release() {
		assert(!size);
		Core::DeferredRelease(heap);
	}

	DescriptorHandle DescriptorHeap::Allocate() {
		std::lock_guard lock{ mutex };
		assert(heap);
		assert(size < capacity);

		const uint32 index{ freeHandles[size] };
		const uint32 offset{ index * descriptorSize };
		++size;

		DescriptorHandle handle;
		handle.CPU.ptr = cpuStart.ptr + offset;
		if (IsShaderVisible()) {
			handle.GPU.ptr = gpuStart.ptr + offset;
		}

		DEBUG_OP(handle.container = this);
		DEBUG_OP(handle.index = index);
		return handle;
	}

	void DescriptorHeap::Free(DescriptorHandle& handle) {
		if (!handle.IsValid())
			return;

		std::lock_guard lock{ mutex };

		assert(heap && size);
		assert(handle.container = this);
		assert(handle.CPU.ptr >= cpuStart.ptr);
		assert((handle.CPU.ptr - cpuStart.ptr) % descriptorSize == 0);
		assert(handle.index < capacity);
		const uint32 index{ (uint32)(handle.CPU.ptr - cpuStart.ptr) / descriptorSize };
		assert(handle.index == index);


		const uint32 frameIndex{Core::CurrentFrameIndex()};
		deferredFreeIndices[frameIndex].PushBack(index);
		Core::SetDeferredReleasesFlag();
		handle = {};
	}
#pragma endregion DescriptorHeap

#pragma region D3D12Texture

	D3D12Texture::D3D12Texture(D3D12TextureInitInfo info) 
	{
		auto* const device{ Core::Device() };
		assert(device);

		D3D12_CLEAR_VALUE* const clearValue
		{
			(info.ResourceDescription &&
			(info.ResourceDescription->Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ||
			info.ResourceDescription->Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
			? &info.ClearValue : nullptr
		};

		if (info.Resource) 
		{
			assert(!info.Heap);
			resource = info.Resource;
		}
		else if (info.Heap && info.ResourceDescription) 
		{
			assert(!info.Resource);
			device->CreatePlacedResource(info.Heap, info.AllocationInfo.Offset, info.ResourceDescription,
				info.InitialState, clearValue, IID_PPV_ARGS(&resource));
		}
		else if(info.ResourceDescription)
		{
			assert(!info.Heap && !info.Resource);
			DXCall(device->CreateCommittedResource(&D3DX::HeapProperties.DefaultHeap, D3D12_HEAP_FLAG_NONE, 
				info.ResourceDescription, info.InitialState, clearValue, IID_PPV_ARGS(&resource)));
		}

		assert(resource);
		srv = Core::GetSRVHeap().Allocate();
		device->CreateShaderResourceView(resource, info.SRVDescription, srv.CPU);
	}

	void D3D12Texture::Release() 
	{
		Core::GetSRVHeap().Free(srv);
		Core::DeferredRelease(resource);
	}
#pragma endregion D3D12Texture
#pragma region D3D12RenderTexture
	D3D12RenderTexture::D3D12RenderTexture(D3D12TextureInitInfo info) 
		: texture{info}
	{
		assert(info.ResourceDescription);
		mipCount = Resource()->GetDesc().MipLevels;
		assert(mipCount && mipCount <= D3D12Texture::MaxMIPS);

		DescriptorHeap& rtvHeap{ Core::GetRTVHeap() };
		D3D12_RENDER_TARGET_VIEW_DESC desc{};
		desc.Format = info.ResourceDescription->Format;
		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
		
		auto* const device{ Core::Device() };
		assert(device);

		for (uint32 i{ 0 }; i < mipCount; i++) 
		{
			rtv[i] = rtvHeap.Allocate();
			device->CreateRenderTargetView(Resource(), &desc, rtv[i].CPU);
			++desc.Texture2D.MipSlice;
		}
	}

	void D3D12RenderTexture::Release() 
	{
		for (uint32 i{ 0 }; i < mipCount; i++)
			Core::GetRTVHeap().Free(rtv[i]);
		texture.Release();
		mipCount = 0;
	}
#pragma endregion D3D12RenderTexture
}