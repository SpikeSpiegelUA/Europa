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
}