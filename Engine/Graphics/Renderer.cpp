#include "Renderer.h"
#include "GraphicsPlatformInterface.h"
#include "Direct3D12/D3D12Interface.h"

namespace Europa::Graphics {
	namespace {

		//Defines where the compiled engine shaders file is located for each one of the supported APIs.
		constexpr const char* EngineShaderPaths[]
		{
			".\\shaders\\d3d12\\shaders.bin",
			// ".\\shaders\\vulkan\\shaders.bin, etc.
		};


		PlatformInterface GFX{};

		bool SetPlatformInterface(GraphicsPlatform platform) {
			switch (platform) {
				case GraphicsPlatform::Direct3D12:
					D3D12::GetPlatformInterface(GFX);
					break;
				default:
					return false;
			}

			assert(GFX.Platform == platform);
			return true;
		}
	}

	bool Initialize(GraphicsPlatform platform) {
		return SetPlatformInterface(platform) && GFX.Initialize();
	}

	void Shutdown() {
		GFX.Shutdown();
	}

	const char* GetEngineShadersPath() 
	{
		return EngineShaderPaths[(uint32)GFX.Platform];
	}

	const char* GetEngineShadersPath(GraphicsPlatform platform)
	{
		return EngineShaderPaths[(uint32)platform];
	}

	Surface CreateSurface(Platform::Window window)
	{
		return GFX.Surface.Create(window);
	}

	void RemoveSurface(SurfaceID id)
	{
		assert(ID::IsValid(id));
		GFX.Surface.Remove(id);
	}

	void Surface::Resize(uint32 width, uint32 height) const
	{
		assert(IsValid());
		GFX.Surface.Resize(ID, width, height);
	}
	const uint32 Surface::Width() const
	{
		assert(IsValid());
		return GFX.Surface.Width(ID);
	}

	const uint32 Surface::Height() const
	{
		assert(IsValid());
		return GFX.Surface.Height(ID);
	}

	void Surface::Render() const
	{
		assert(IsValid());
		GFX.Surface.Render(ID);
	}
}