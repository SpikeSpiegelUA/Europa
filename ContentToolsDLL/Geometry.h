#pragma once

#include "ToolsCommon.h"

namespace Europa::Tools {

	struct Mesh {
		Utilities::Vector<Math::Vector3> Positions;
		Utilities::Vector<Math::Vector3> Normals;
		Utilities::Vector<Math::Vector4> Tangents;
		Utilities::Vector<Utilities::Vector<Math::Vector2>> UVSets;

		Utilities::Vector<uint32> RawIndices;
	};

	struct LODGroup {
		std::string Name;
		Utilities::Vector<Mesh> Meshes;
	};

	struct Scene {
		std::string Name;
		Utilities::Vector<LODGroup> LODGroups;
	};

	struct GeometryImportSettings {
		float32 SmoothingAngle;
		uint8 CalculateNormals;
		uint8 CalculateTangents;
		uint8 ReverseHandedness;
		uint8 ImportEmbededTextures;
		uint8 ImportTextures;
	};

	struct SceneData {
		uint8* Buffer;
		uint32 BufferSize;
		GeometryImportSettings Settings;
	};

}
