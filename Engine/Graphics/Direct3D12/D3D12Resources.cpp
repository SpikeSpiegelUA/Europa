#include "D3D12Resources.h"
#include "D3D12Core.h"

namespace Europa::Graphics::D3D12 {

	bool DescriptorHeap::Initialize(uint32 capacity, bool isShaderVisible) {
		std::lock_guard lock{ mutex };
		assert(capacity && capacity < D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2);
		assert(!(type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER && 
			capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE));
		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV || type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV) {
			isShaderVisible = false;
		}

		Release();

		ID3D12Device* const device{ Core::Device() };
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

		descriptorSize = device->GetDescriptorHandleIncrementSize(type);
		cpuStart = heap->GetCPUDescriptorHandleForHeapStart();
		gpuStart = isShaderVisible ? heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };

		return true;
	}

	void DescriptorHeap::Release() {

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
		deferredFreeIndices[frameIndex].push_back(index);
		Core::SetDeferredReleasesFlag();
		handle = {};
	}

}