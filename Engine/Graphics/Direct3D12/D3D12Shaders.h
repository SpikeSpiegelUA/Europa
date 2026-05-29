#pragma once
#include "D3D12CommonHeaders.h"

namespace Europa::Graphics::D3D12::Shaders {
	struct ShaderType {
		enum Type : uint32 {
			Vertex = 0,
			Hull,
			Domain,
			Geometry,
			Pixel,
			Compute,
			Amplification,
			Mesh,

			Count
		};
	};

	struct EngineShader {
		enum ID : uint32 {
			FullscreenTriangleVS = 0,
			FillColorPS = 1,
			PostProcessPS = 2,
			Count
		};
	};

	bool Initialize();
	void Shutdown();

	D3D12_SHADER_BYTECODE GetEngineShader(EngineShader::ID id);
}