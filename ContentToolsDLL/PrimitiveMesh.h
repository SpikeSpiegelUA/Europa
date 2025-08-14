#pragma once

#include "ToolsCommon.h"

namespace Europa::Tools {
	enum PrimitiveMeshType : uint32 {
		Plane,
		Cube,
		UVSphere,
		Icosphere,
		Cylinder,
		Capsule,
		Count
	};

	struct PrimitiveInitInfo {
		PrimitiveMeshType Type;
		uint32 Segments[3]{ 1, 1, 1 };
		Math::Vector3 Size{ 1, 1, 1 };
		uint32 LOD{ 0 };
	};
}
