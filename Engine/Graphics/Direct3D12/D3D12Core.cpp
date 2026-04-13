#include "D3D12Core.h"
#include "D3D12Resources.h"
using namespace Microsoft::WRL;

namespace Europa::Graphics::D3D12::Core {
	namespace {
		bool FailedInit() {
			Shutdown();
			return false;
		}

		class D3D12Command {
		public:
			D3D12Command() = default;
			DISABLE_COPY_AND_MOVE(D3D12Command);
			explicit D3D12Command(ID3D12Device8* const device, D3D12_COMMAND_LIST_TYPE type) {
				HRESULT hr{ S_OK };
				D3D12_COMMAND_QUEUE_DESC desc{};
				desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
				desc.NodeMask = 0;
				desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
				desc.Type = type;
				DXCall(hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&cmdQueue)));
				if (FAILED(hr))
					goto _error;
				NAME_D3D12_OBJECT(cmdQueue, type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"GFX Command Queue" :
					type == D3D12_COMMAND_LIST_TYPE_COMPUTE ? L"Compute Command Queue" : L"Command Queue");
				for (uint32 i{ 0 }; i < FrameBufferCount; ++i) {
					CommandFrame& frame{ commandFrames[i] };
					DXCall(hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&frame.CMDAllocator)));
					if (FAILED(hr)) goto _error;

					NAME_D3D12_OBJECT_INDEXED(frame.CMDAllocator, i, type == D3D12_COMMAND_LIST_TYPE_DIRECT
						? L"GFX Command Allocator" : type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
						L"Compute Command Allocator" : L"Command Allocator");
				}
				DXCall(hr = device->CreateCommandList(0, type, commandFrames[0].CMDAllocator, nullptr, IID_PPV_ARGS(&cmdList)));
				if (FAILED(hr)) goto _error;
				DXCall(cmdList->Close());
				NAME_D3D12_OBJECT(cmdList, type == D3D12_COMMAND_LIST_TYPE_DIRECT ? L"GFX Command List" :
					type == D3D12_COMMAND_LIST_TYPE_COMPUTE ? L"Compute Command List" : L"Command List");
				DXCall(hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
				if (FAILED(hr))
					goto _error;
				NAME_D3D12_OBJECT(fence, L"D3D12 Fence");

				fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
				assert(fenceEvent);

				return;

			_error:
				Release();
			}

			~D3D12Command() {
				assert(!cmdQueue && !cmdList && !fence);
			}

			void BeginFrame() {
				CommandFrame& frame{ commandFrames[frameIndex] };
				frame.Wait(fenceEvent, fence);
				DXCall(frame.CMDAllocator->Reset());
				DXCall(cmdList->Reset(frame.CMDAllocator, nullptr));
			}

			void EndFrame() {
				DXCall(cmdList->Close());
				ID3D12CommandList* const cmdlists[]{ cmdList };
				cmdQueue->ExecuteCommandLists(_countof(cmdlists), &cmdlists[0]);
				uint64& fenceValue{ fenceValue };
				++fenceValue;
				CommandFrame& frame{ commandFrames[frameIndex] };
				frame.FenceValue = fenceValue;
				cmdQueue->Signal(fence, fenceValue);
				frameIndex = (frameIndex + 1) % FrameBufferCount;
			}

			void Flush() {
				for (uint32 i{ 0 }; i < FrameBufferCount; i++) {
					commandFrames[i].Wait(fenceEvent, fence);
				}
				frameIndex = 0;
			}

			void Release() {
				Flush();
				Core::Release(fence);
				fenceValue = 0;

				CloseHandle(fenceEvent);
				fenceEvent = nullptr;

				Core::Release(cmdQueue);
				Core::Release(cmdList);

				for (uint32 i{ 0 }; i < FrameBufferCount; i++) {
					commandFrames[i].Release();
				}
			}

			constexpr ID3D12CommandQueue* const CommandQueue() const {
				return cmdQueue;
			}
			constexpr ID3D12GraphicsCommandList6* const СommandList() const {
				return cmdList;
			}
			constexpr uint32 FrameIndex() const {
				return frameIndex;
			}
		private:

			struct CommandFrame {
				ID3D12CommandAllocator* CMDAllocator{ nullptr };
				uint64 FenceValue{ 0 };

				void Wait(HANDLE fenceEvent, ID3D12Fence1* fence) {
					assert(fence && fenceEvent);

					if (fence->GetCompletedValue() < FenceValue) {
						DXCall(fence->SetEventOnCompletion(FenceValue, fenceEvent));
						WaitForSingleObject(fenceEvent, INFINITE);
					}
				}

				void Release() {
					Core::Release(CMDAllocator);
					FenceValue = 0;
				}
			};

			ID3D12CommandQueue* cmdQueue{ nullptr };
			ID3D12GraphicsCommandList6* cmdList{ nullptr };
			ID3D12Fence1* fence{};
			HANDLE fenceEvent{};
			uint64 fenceValue{};
			CommandFrame commandFrames[FrameBufferCount]{};
			uint32 frameIndex{ 0 };
		};

		ID3D12Device14* MainDevice{ nullptr };
		IDXGIFactory7* DXGIFactory{ nullptr };
		D3D12Command GFXCommand;

		DescriptorHeap rtvDescriptorHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV };
		DescriptorHeap dsvDescriptorHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV };
		DescriptorHeap srvDescriptorHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };
		DescriptorHeap uavDescriptorHeap{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };

		Utilities::Vector<IUnknown*> DeferredReleases[FrameBufferCount]{};
		uint32 DeferredReleasesFlag[FrameBufferCount]{};
		std::mutex DeferredReleasesMutex{};

		constexpr DXGI_FORMAT RenderTargetFormat{ DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
		constexpr D3D_FEATURE_LEVEL MinimumFeatureLevel{ D3D_FEATURE_LEVEL_11_0 };

		void __declspec(noinline) ProcessDeferredReleases(uint32 frameIndex) {
			std::lock_guard lock{ DeferredReleasesMutex };

			//Note: we clear this flag in the beginning. If we'd clear it at the end. then it might
			//overwrite some other thread that was trying to set it.
			//It's fine if overwriting happens before processing the items.
			DeferredReleasesFlag[frameIndex] = 0;

			rtvDescriptorHeap.ProcessDeferredFree(frameIndex);
			dsvDescriptorHeap.ProcessDeferredFree(frameIndex);
			srvDescriptorHeap.ProcessDeferredFree(frameIndex);
			uavDescriptorHeap.ProcessDeferredFree(frameIndex);

			Utilities::Vector<IUnknown*>& resources{DeferredReleases[frameIndex]};
			if (!resources.empty()) {
				for (auto& resource : resources)
					Release(resource);
				resources.clear();
			}
		}

		IDXGIAdapter4* DetermineMainAdapter() {
			IDXGIAdapter4* adapter{ nullptr };

			for (uint32 i{ 0 }; DXGIFactory->EnumAdapterByGpuPreference(i,
				DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				if (SUCCEEDED(D3D12CreateDevice(adapter, MinimumFeatureLevel, __uuidof(ID3D12Device), nullptr))) {
					return adapter;
				}

				Release(adapter);
			}

			return nullptr;

		}

		D3D_FEATURE_LEVEL GetMaxFeatureLevel(IDXGIAdapter4* adapter) {
			constexpr D3D_FEATURE_LEVEL featureLevels[4]{
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_12_0,
				D3D_FEATURE_LEVEL_12_1,
			};

			D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevelInfo{};
			featureLevelInfo.NumFeatureLevels = _countof(featureLevels);
			featureLevelInfo.pFeatureLevelsRequested = featureLevels;

			ComPtr<ID3D12Device> device;
			DXCall(D3D12CreateDevice(adapter, MinimumFeatureLevel, IID_PPV_ARGS(&device)));
			DXCall(device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevelInfo, sizeof(featureLevelInfo)));
			return featureLevelInfo.MaxSupportedFeatureLevel;
		}
	}//Anonymous namespace

	namespace Details {
		void DeferredRelease(IUnknown* resource)
		{
			const uint32 frameIndex{ CurrentFrameIndex() };
			std::lock_guard lock{ DeferredReleasesMutex };
			DeferredReleases[frameIndex].push_back(resource);
			SetDeferredReleasesFlag();
		}
	}//Details namespace

	bool Initialize() {
		if (MainDevice)
			Shutdown();
		uint32 DXGIFactoryFlags{ 0 };
#ifdef _DEBUG
		{
			ComPtr<ID3D12Debug3> debugInterface;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
				debugInterface->EnableDebugLayer();
			else
				OutputDebugStringA("Warning: D3D12 Debug interface is not available. Verify that Graphics\
					Tools optional feature is installed in this system.\n");
			DXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
#endif

		HRESULT hr{ S_OK };
		DXCall(hr = CreateDXGIFactory2(DXGIFactoryFlags, IID_PPV_ARGS(&DXGIFactory)));
		if (FAILED(hr))
			return FailedInit();

		ComPtr<IDXGIAdapter4> mainAdapter;
		mainAdapter.Attach(DetermineMainAdapter());
		if (!mainAdapter)
			return FailedInit();

		D3D_FEATURE_LEVEL maxFeatureLevel{ GetMaxFeatureLevel(mainAdapter.Get()) };
		assert(maxFeatureLevel >= MinimumFeatureLevel);
		if (maxFeatureLevel < MinimumFeatureLevel)
			return FailedInit();

		DXCall(hr = D3D12CreateDevice(mainAdapter.Get(), maxFeatureLevel, IID_PPV_ARGS(&MainDevice)));
		if (FAILED(hr))
			return FailedInit();

#ifdef _DEBUG
		{
			ComPtr<ID3D12InfoQueue> infoQueue;
			DXCall(MainDevice->QueryInterface(IID_PPV_ARGS(&infoQueue)));

			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		}
#endif

		bool result{ true };
		result &= rtvDescriptorHeap.Initialize(512, false);
		result &= dsvDescriptorHeap.Initialize(512, false);
		result &= srvDescriptorHeap.Initialize(4096, true);
		result &= uavDescriptorHeap.Initialize(512, false);

		if (!result)
			return FailedInit();

		new (&GFXCommand) D3D12Command(MainDevice, D3D12_COMMAND_LIST_TYPE_DIRECT);
		if (!GFXCommand.CommandQueue())
			return FailedInit();

		NAME_D3D12_OBJECT(MainDevice, L"Main D3D12 Device");
		NAME_D3D12_OBJECT(rtvDescriptorHeap.Heap(), L"RTV Descriptor Heap");
		NAME_D3D12_OBJECT(dsvDescriptorHeap.Heap(), L"DSV Descriptor Heap");
		NAME_D3D12_OBJECT(srvDescriptorHeap.Heap(), L"SRV Descriptor Heap");
		NAME_D3D12_OBJECT(uavDescriptorHeap.Heap(), L"UAV Descriptor Heap");

		return true;
	}

	void Shutdown() {
		GFXCommand.Release();

		//Note: we don't call ProcessDeferredReleases at the end because some resouces
		//(such as swap chains) can't be released before their depending resources are released.
		for (uint32 i{ 0 }; i < FrameBufferCount; ++i)
			ProcessDeferredReleases(i);

		Release(DXGIFactory);

		rtvDescriptorHeap.Release();
		dsvDescriptorHeap.Release();
		srvDescriptorHeap.Release();
		uavDescriptorHeap.Release();

		//Note: some types only use deferred release for their resources during shutdown/reset/clear.
		//To finally release these resources we call ProcessDeferredReleases once more.
		ProcessDeferredReleases(0);
#ifdef _DEBUG
		{
			{
				ComPtr<ID3D12InfoQueue> infoQueue;
				DXCall(MainDevice->QueryInterface(IID_PPV_ARGS(&infoQueue)));

				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
			}
			ComPtr<ID3D12DebugDevice2> debugDevice;
			DXCall(MainDevice->QueryInterface(IID_PPV_ARGS(&debugDevice)));
			Release(MainDevice);
			DXCall(debugDevice->ReportLiveDeviceObjects(
				D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL));

		}
#endif
		Release(MainDevice);
	}

	void Render() {
		GFXCommand.BeginFrame();
		ID3D12GraphicsCommandList* CMDList{GFXCommand.СommandList()};
		const uint32 frameIndex{ CurrentFrameIndex() };
		if (DeferredReleasesFlag[frameIndex]) {
			ProcessDeferredReleases(frameIndex);
		}
		GFXCommand.EndFrame();
	}

	ID3D12Device* const Device()
	{
		return MainDevice;
	}
	DescriptorHeap& GetRTVHeap()
	{
		return rtvDescriptorHeap;
	}
	DescriptorHeap& GetDSVHeap()
	{
		return dsvDescriptorHeap;
	}
	DescriptorHeap& GetSRVHeap()
	{
		return srvDescriptorHeap;
	}
	DescriptorHeap& GetUAVHeap()
	{
		return uavDescriptorHeap;
	}

	uint32 CurrentFrameIndex()
	{
		return GFXCommand.FrameIndex();
	}
	DXGI_FORMAT GetDefaultRenderTargetFormat()
	{
		return RenderTargetFormat;
	}
	void SetDeferredReleasesFlag()
	{
		DeferredReleasesFlag[CurrentFrameIndex()] = 1;
	}
}
