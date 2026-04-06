#pragma once
#include "../Common/CommonHeaders.h"
#include "..\Platform\Window.h"

namespace Europa::Graphics {
	class Surface {

	};

	struct RenderSurface {
		Platform::Window Window;
		Surface Surface;
	};

	enum class GraphicsPlatform : uint32 {
		Direct3D12 = 0,
	};

	bool Initialize(GraphicsPlatform platform);
	void Shutdown();
	void Render();
}