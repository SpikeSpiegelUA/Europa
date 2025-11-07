#pragma once

#include "ToolsCommon.h"

namespace Europa::Tools {

	namespace PackedVertex {
		struct VertexStatic {
			Math::Vector3 Position;
			uint8 Reserved[3];
			uint8 TSign;
			uint16 Normal[2];
			uint16 Tangent[2];
			Math::Vector2 UV;
		};
	}

	struct Vertex {
		Math::Vector3 Position;
		Math::Vector3 Normal;
		Math::Vector4 Tangent;
		Math::Vector2 UV;
	};

	struct Mesh {
		//Initial data.
		Utilities::Vector<Math::Vector3> Positions;
		Utilities::Vector<Math::Vector3> Normals;
		Utilities::Vector<Math::Vector4> Tangents;
		Utilities::Vector<Utilities::Vector<Math::Vector2>> UVSets;

		Utilities::Vector<uint32> RawIndices;

		//Intermediate data.
		Utilities::Vector<Vertex> Vertices;
		Utilities::Vector<uint32> Indices;

		//Output data
		std::string Name;
		Utilities::Vector<PackedVertex::VertexStatic> PackedVerticesStatic;
		float32 LODThreshold{ -1.f };
		uint32 LODID{ uint32_invalid_id };
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

	void ProcessScene(Scene& scene, const GeometryImportSettings& settings);
	void PackData(const Scene& scene, SceneData& data);

}
