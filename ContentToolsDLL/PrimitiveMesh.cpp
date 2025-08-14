#include "PrimitiveMesh.h"
#include "Geometry.h"

namespace Europa::Tools {
	namespace {

		using namespace Math;
		using PrimitiveMeshCreator = void(*)(Scene&, const PrimitiveInitInfo& info);

		void CreatePlane(Scene&, const PrimitiveInitInfo& info);
		void CreateCube(Scene&, const PrimitiveInitInfo& info);
		void CreateUVSphere(Scene&, const PrimitiveInitInfo& info);
		void CreateIcosphere(Scene&, const PrimitiveInitInfo& info);
		void CreateCylinder(Scene&, const PrimitiveInitInfo& info);
		void CreateCapsule(Scene&, const PrimitiveInitInfo& info);

		PrimitiveMeshCreator Creators[]{
			CreatePlane,
			CreateCube,
			CreateUVSphere,
			CreateIcosphere,
			CreateCylinder,
			CreateCapsule
		};

		static_assert(_countof(Creators) == PrimitiveMeshType::Count);

		struct Axis {
			enum  : uint32 {
				X = 0,
				Y = 1,
				Z = 2
			};
		};

		Mesh CreatePlane(const PrimitiveInitInfo& info, uint32 horizontalIndex = Axis::X, uint32 verticalIndex = Axis::Z, bool flipWinding = false,
			Vector3 offset = { -0.5f, 0.f, -0.5f }, Vector2 uRange = { 0.f, 1.f }, Vector2 vRange = { 0.f, 1.f }) 
		{
			assert(horizontalIndex < 3 && verticalIndex < 3);
			assert(horizontalIndex != verticalIndex);

			const uint32 horizontalCount{ Clamp(info.Segments[horizontalIndex], 1u, 10u) };
			const uint32 verticalCount{ Clamp(info.Segments[verticalIndex], 1u, 10u) };
			const float32 horizontalStep{ 1.f / horizontalCount };
			const float32 verticalStep{ 1.f / verticalCount };
			const float32 uStep{ (uRange.y - uRange.x) / horizontalCount };
			const float32 vStep{ (vRange.y - vRange.x) / verticalCount };

			Mesh mesh;
			Utilities::Vector<Vector2> uvs;

			for(uint32 j{0}; j <= verticalCount; ++j)
				for (uint32 i{ 0 }; i <= horizontalCount; ++i) {
					Vector3 position{ offset };
					float32* const asArray{ &position.x };
					asArray[horizontalIndex] += i * horizontalStep;
					asArray[verticalIndex] += j * verticalStep;
					mesh.Positions.emplace_back(position.x * info.Size.x, position.y * info.Size.y, position.z * info.Size.z);

					Vector2 uv{ uRange.x, 1.f - vRange.x };
					uv.x += i * uStep;
					uv.y -= j * vStep;
					uvs.emplace_back(uv);
				}

			const uint32 rowLength{ horizontalCount + 1 };
			for (uint32 j{ 0 }; j < verticalCount; ++j) {
				uint32 k{ 0 };
				for (uint32 i{ k }; i < horizontalCount; ++i) {
					const uint32 index[4]{
						i + j * rowLength,
						i + (j + 1) * rowLength,
						(i + 1) + j * rowLength,
						(i + 1) + (j + 1) * rowLength

					};

					mesh.RawIndices.emplace_back(index[0]);
					mesh.RawIndices.emplace_back(index[flipWinding ? 2 : 1]);
					mesh.RawIndices.emplace_back(index[flipWinding ? 1 : 2]);

					mesh.RawIndices.emplace_back(index[2]);
					mesh.RawIndices.emplace_back(index[flipWinding ? 3 : 1]);
					mesh.RawIndices.emplace_back(index[flipWinding ? 1 : 3]);
				}
				++k;
			}
			const uint32 numIndices{ 3 * 2 * horizontalCount * verticalCount };
			assert(mesh.RawIndices.size() == numIndices);

			for (uint32 i{ 0 }; i < numIndices; ++i) {
				mesh.UVSets[0].emplace_back(uvs[mesh.RawIndices[i]]);
			}
		}

		void CreatePlane(Scene& scene, const PrimitiveInitInfo& info) {
			LODGroup lodGroup;
			lodGroup.Name = "Plane";
			lodGroup.Meshes.emplace_back(CreatePlane(info));
			scene.LODGroups.emplace_back(lodGroup);
		}

		void CreateCube(Scene& scene, const PrimitiveInitInfo& info) {

		}

		void CreateUVSphere(Scene& scene, const PrimitiveInitInfo& info) {

		}

		void CreateIcosphere(Scene& scene, const PrimitiveInitInfo& info) {

		}

		void CreateCylinder(Scene& scene, const PrimitiveInitInfo& info) {

		}

		void CreateCapsule(Scene& scene, const PrimitiveInitInfo& info) {

		}

	}
	EDITOR_INTERFACE void CreatePrimitiveMesh(SceneData* data, PrimitiveInitInfo* info) {
		assert(data && info);
		assert(info->Type < PrimitiveMeshType::Count);
		Scene scene;
		Creators[info->Type](scene, *info);

		data->Settings.CalculateNormals = 1;
		ProcessScene(scene, data->Settings);
		PackData(scene, *data);
	}
}