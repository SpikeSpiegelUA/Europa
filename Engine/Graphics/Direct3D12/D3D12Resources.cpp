#include "D3D12Resources.h"
#include "D3D12Core.h"

namespace Europa::Graphics::D3D12 {

	bool DescriptorHeap::Initialize(uint32 capacity, bool isShaderVisible) {
		std::lock_guard lock{ mutex };
		assert(capacity && capacity < D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2);
		assert(!(type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER && 
			capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE));
		ID3D12Device* const device{ Core::Device() };
		assert(device);
	}

	void DescriptorHeap::Release() {

	}

	DescriptorHandle DescriptorHeap::Allocate() {
		std::lock_guard lock{ mutex };
	}

	void DescriptorHeap::Free(DescriptorHandle& handle) {
		std::lock_guard lock{ mutex };
	}

}