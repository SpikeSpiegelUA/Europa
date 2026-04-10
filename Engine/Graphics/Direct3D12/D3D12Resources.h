#pragma once

#include "D3D12CommonHeaders.h"

namespace Europa::Graphics::D3D12{

	class DescriptorHeap;

	struct DescriptorHandle {
		D3D12_CPU_DESCRIPTOR_HANDLE CPU{};
		D3D12_GPU_DESCRIPTOR_HANDLE GPU{};

		constexpr bool IsValid() const 
		{
			return CPU.ptr != 0; 
		}

		constexpr bool IsShaderVisible() const 
		{ 
			return GPU.ptr != 0; 
		}

#ifdef _DEBUG
	private:
		friend class DescriptorHeap;
		DescriptorHeap* container{ nullptr };
		uint32 index{ uint32_invalid_id };
	};
#endif

	class DescriptorHeap {
	public:
		explicit DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) : type{type} {}
		DISABLE_COPY_AND_MOVE(DescriptorHeap);
		~DescriptorHeap() {
			assert(!heap);
		}

		bool Initialize(uint32 capacity, bool isShaderVisible);
		void Release();

		[[nodiscard]] DescriptorHandle Allocate();
		void Free(DescriptorHandle& handle);

		constexpr D3D12_DESCRIPTOR_HEAP_TYPE Type() const 
		{ 
			return type; 
		}

		constexpr D3D12_CPU_DESCRIPTOR_HANDLE CPUStart() const {
			return cpuStart;
		}

		constexpr D3D12_GPU_DESCRIPTOR_HANDLE GPUStart() const {
			return gpuStart;
		}

		constexpr ID3D12DescriptorHeap* const Heap() const {
			return heap;
		}

		constexpr uint32 Capacity() const {
			return capacity;
		}

		constexpr uint32 Size() const {
			return size;
		}

		constexpr uint32 DescriptorSize() const {
			return descriptorSize;
		}

		constexpr bool IsShaderVisible() const {
			return gpuStart.ptr != 0;
		}

	private:
		ID3D12DescriptorHeap* heap{};
		D3D12_CPU_DESCRIPTOR_HANDLE cpuStart{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpuStart{};
		std::unique_ptr<uint32[]> freeHandles{};
		uint32 capacity{ 0 };
		std::vector<uint32> deferredFreeIndices[FrameBufferCount];
		std::mutex mutex;
		uint32 size{ 0 };
		uint32 descriptorSize{};
		const D3D12_DESCRIPTOR_HEAP_TYPE type{};
	};
}
