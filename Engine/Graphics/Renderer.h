#pragma once
#include "../Common/CommonHeaders.h"
#include "..\Platform\Window.h"

namespace Europa::Graphics {
	DEFINE_TYPED_ID(SurfaceID);
	class Surface {
	public:
		constexpr explicit Surface(SurfaceID id) : ID{ id } {}
		constexpr Surface() = default;
		constexpr SurfaceID GetID() const { return ID; }
		constexpr bool IsValid() const { return ID::IsValid(ID); }

		void Resize(uint32 width, uint32 height) const;
		const uint32 Width() const;
		const uint32 Height() const;
		void Render() const;
	private:
		SurfaceID ID{ ID::InvalidID };
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

	Surface CreateSurface(Platform::Window window);
	void RemoveSurface(SurfaceID id);
}