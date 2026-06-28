#include "D3D12GPass.h"
#include "D3D12Core.h"
#include "D3D12Shaders.h"

namespace Europa::Graphics::D3D12::GPass {
	namespace {
		struct GPassRootParamIndices {
			enum : uint32 {
				RootConstants,
				Count
			};
		};

		constexpr Math::UInt32Vector2 InitialDimensions{ 100, 100 };
		constexpr DXGI_FORMAT MainBufferFormat{ DXGI_FORMAT_R16G16B16A16_FLOAT };
		constexpr DXGI_FORMAT DepthBufferFormat{ DXGI_FORMAT_D32_FLOAT };

		D3D12RenderTexture GPassMainBuffer{};
		D3D12DepthBuffer GPassDepthBuffer{};
		Math::UInt32Vector2 Dimensions{ InitialDimensions };
		D3D12_RESOURCE_BARRIER_FLAGS Flags{};

		ID3D12RootSignature* GPassRootSignature{ nullptr };
		ID3D12PipelineState* GPassPSO{ nullptr };

#if _DEBUG
		constexpr float32 ClearValue[4]{ 0.5f,0.5f,0.5f,1.f };
#else
		constexpr float32 ClearValue[4]{};
#endif

		bool CreateBuffers(Math::UInt32Vector2 size) {
			assert(size.x && size.y);
			GPassMainBuffer.Release();
			GPassDepthBuffer.Resource();

			D3D12_RESOURCE_DESC desc{};
			desc.Alignment = 0; // NOTE: 0 is the same as 64 KB(or 4MB for MSAA)
			desc.DepthOrArraySize = 1;
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			desc.Format = MainBufferFormat;
			desc.Height = size.y;
			desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			desc.MipLevels = 0; // Make space for all mip levels
			desc.SampleDesc = { 1, 0 };
			desc.Width = size.x;

			//Create the main buffer
			{
				D3D12TextureInitInfo info{};
				info.ResourceDescription = &desc;
				info.ResourceDescription = &desc;
				info.InitialState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
				info.ClearValue.Format = desc.Format;
				memcpy(&info.ClearValue.Color, &ClearValue[0], sizeof(ClearValue));
				GPassMainBuffer = D3D12RenderTexture{ info };
			}

			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			desc.Format = DepthBufferFormat;
			desc.MipLevels = 1;

			//Create the depth buffer
			{
				D3D12TextureInitInfo info{};
				info.ResourceDescription = &desc;
				info.InitialState = D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
					D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
				info.ClearValue.Format = desc.Format;
				info.ClearValue.DepthStencil.Depth = 0.f;
				info.ClearValue.DepthStencil.Stencil = 0;

				GPassDepthBuffer = D3D12DepthBuffer{ info };
			}

			NAME_D3D12_OBJECT(GPassMainBuffer.Resource(), L"GPass Main Buffer");
			NAME_D3D12_OBJECT(GPassDepthBuffer.Resource(), L"GPass Depth Buffer");

			Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

			return GPassMainBuffer.Resource() && GPassDepthBuffer.Resource();
		}

		bool CreateGPassPSOAndRootSignature()
		{
			assert(!GPassRootSignature && !GPassPSO);

			//Create GPass root signature
			using IDX = GPassRootParamIndices;
			D3DX::D3D12RootParameter parameters[IDX::Count]{};
			parameters[0].AsConstants(3, D3D12_SHADER_VISIBILITY_PIXEL, 1);
			const D3DX::D3D12RootSignatureDescription rootSignature{ &parameters[0], IDX::Count };
			GPassRootSignature = rootSignature.Create();
			assert(GPassRootSignature);
			NAME_D3D12_OBJECT(GPassRootSignature, L"GPass Root Signature");
			//Create GPass PSO

			struct {
				D3DX::D3D12PipelineStateSubobjectRootSignature rootSignature{ GPassRootSignature };
				D3DX::D3D12PipelineStateSubobjectVS vs{ Shaders::GetEngineShader(Shaders::EngineShader::FullscreenTriangleVS) };
				D3DX::D3D12PipelineStateSubobjectPS ps{ Shaders::GetEngineShader(Shaders::EngineShader::FillColorPS) };
				D3DX::D3D12PipelineStateSubobjectPrimitiveTopology primitiveTopology{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE };
				D3DX::D3D12PipelineStateSubobjectRenderTargetFormats renderTargetFormats;
				D3DX::D3D12PipelineStateSubobjectDepthStencilFormat depthStencilFormat{};
				D3DX::D3D12PipelineStateSubobjectRasterizer rasterizer{D3DX::RasterizerState.NoCulling};
				D3DX::D3D12PipelineStateSubobjectDepthStencil1 depth{D3DX::DepthState.Disabled};
			} stream;

			D3D12_RT_FORMAT_ARRAY rtfArray{};
			rtfArray.NumRenderTargets = 1;
			rtfArray.RTFormats[0] = MainBufferFormat;

			stream.renderTargetFormats = rtfArray;

			GPassPSO = D3DX::CreatePipelineState(&stream, sizeof(stream));
			NAME_D3D12_OBJECT(GPassPSO, L"GPass Pipeline State Object");

			return GPassRootSignature && GPassPSO;
		}
	}//Anonymous namespace

	bool Initialize() 
	{
		return CreateBuffers(InitialDimensions) && CreateGPassPSOAndRootSignature();
	}

	void Shutdown() {
		GPassMainBuffer.Release();
		GPassDepthBuffer.Release();
		Dimensions = InitialDimensions;

		Core::Release(GPassRootSignature);
		Core::Release(GPassPSO);
	}

	[[nodiscard]] const D3D12RenderTexture& MainBuffer()
	{
		return GPassMainBuffer;
	}
	[[nodiscard]] const D3D12DepthBuffer& DepthBuffer() 
	{
		return GPassDepthBuffer;
	}

	void SetSize(Math::UInt32Vector2 size) 
	{
		Math::UInt32Vector2& d{ Dimensions };
		if (size.x > d.x || size.y > d.y) 
		{
			d = { std::max(size.x, d.x), std::max(size.y,d.y) };
			CreateBuffers(d);
		}
	}


	void DepthPrepass(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& info) {

	}
	void Render(ID3D12GraphicsCommandList* cmdList, const D3D12FrameInfo& info) {
		cmdList->SetGraphicsRootSignature(GPassRootSignature);
		cmdList->SetPipelineState(GPassPSO);

		static uint32 frame{ 0 };
		struct {
			float32 width;
			float32 height;
			uint32 frame;
		} constants { (float32)info.SurfaceWidth, (float32)info.SurfaceHeight, ++frame};

		using IDX = GPassRootParamIndices;
		cmdList->SetGraphicsRoot32BitConstants(IDX::RootConstants, 3, &constants, 0);

		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->DrawInstanced(3, 1, 0, 0);
	}

	void AddTransitionsForDepthPrepass(D3DX::D3D12ResourceBarrier& barriers) 
	{
		barriers.Add(GPassMainBuffer.Resource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY);
		barriers.Add(GPassDepthBuffer.Resource(), D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE, Flags);

		Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
	}
	void AddTransitionsForGPass(D3DX::D3D12ResourceBarrier& barriers) 
	{
		barriers.Add(GPassMainBuffer.Resource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_FLAG_END_ONLY);
		barriers.Add(GPassDepthBuffer.Resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ |
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
	void AddTransitionsForPostProcess(D3DX::D3D12ResourceBarrier& barriers) 
	{
		barriers.Add(GPassMainBuffer.Resource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		barriers.Add(GPassDepthBuffer.Resource(), D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY);
	}

	void SetRenderTargetsForDepthPrepass(ID3D12GraphicsCommandList* cmdList) 
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE dsv{ GPassDepthBuffer.DSV()};
		cmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 0.f, 0, 0, nullptr);
		cmdList->OMSetRenderTargets(0, nullptr, 0, &dsv);
	}
	void SetRenderTargetsForGPass(ID3D12GraphicsCommandList* cmdList) 
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE rtv{ GPassMainBuffer.RTV(0) };
		const D3D12_CPU_DESCRIPTOR_HANDLE dsv{ GPassDepthBuffer.DSV() };

		cmdList->ClearRenderTargetView(rtv, ClearValue, 0, nullptr);
		cmdList->OMSetRenderTargets(1, &rtv, 0, &dsv);
	}
}