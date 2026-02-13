#include "PrimitiveMesh.h"
#include "Geometry.h"

namespace Europa::Tools {
	namespace {

		using namespace Math;
		using namespace DirectX;
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

			mesh.UVSets.resize(1);

			for (uint32 i{ 0 }; i < numIndices; ++i) {
				mesh.UVSets[0].emplace_back(uvs[mesh.RawIndices[i]]);
			}

			return mesh;
		}

		Mesh CreateUVSphere(const PrimitiveInitInfo& info) {
			const uint32 phi_count{ Clamp(info.Segments[Axis::X], 3u, 64u) };
			const uint32 theta_count{ Clamp(info.Segments[Axis::Y], 2u, 64u) };
			const float32 theta_step{ PI / theta_count };
			const float32 phi_step{ TwoPI / phi_count };
			const uint32 num_indices{ 2 * 3 * phi_count + 2 * 3 * phi_count * (theta_count - 2) };
			const uint32 num_vertices{ 2 + phi_count * (theta_count - 1) };

			Mesh m;
			m.Name = "UVSphere";
			m.Positions.resize(num_vertices);

			//Add the top vertex.
			uint32 c{ 0 };
			m.Positions[c++] = { 0.f, info.Size.y, 0.f };

			for (uint32 i{ 1 }; i <= (theta_count - 1); ++i) {
				const float32 theta{ i * theta_step };
				for (uint32 j{ 0 }; j < phi_count; ++j) {
					const float32 phi{ j * phi_step };
					m.Positions[c++] = {
						info.Size.x * XMScalarSin(theta) * XMScalarCos(phi),
						info.Size.y * XMScalarCos(theta),
						-info.Size.z * XMScalarSin(theta) * XMScalarSin(phi)
					};
				}
			}

			//Add the bottom vertex.
			m.Positions[c++] = { 0.f, -info.Size.y, 0.f };
			assert(c == num_vertices);

			c = 0;
			m.RawIndices.resize(num_indices);
			Utilities::Vector<Vector2> uvs(num_indices);
			const float32 inv_theta_count{ 1.f / theta_count };
			const float32 inv_phi_count{ 1.f / phi_count };

			//Indices for the first cap, connecting the north pole to the first ring
			for (uint32 i{ 0 }; i < phi_count - 1; i++) {
				uvs[c] = { (2 * i + 1) * 0.5f * inv_phi_count, 1.f };
				m.RawIndices[c++] = 0;
				uvs[c] = { i * inv_phi_count, 1.f - inv_theta_count };
				m.RawIndices[c++] = i + 1;
				uvs[c] = { (i + 1) * inv_phi_count, 1.f - inv_theta_count };
				m.RawIndices[c++] = i + 2;
			}

			uvs[c] = { 1.f - 0.5 * inv_phi_count, 1.f };
			m.RawIndices[c++] = 0;
			uvs[c] = { 1.f - inv_phi_count, 1.f - inv_theta_count };
			m.RawIndices[c++] = phi_count;
			uvs[c] = { 1.f, 1.f - inv_theta_count };
			m.RawIndices[c++] = 1;

			//Indices for the section between the top and bottom rings
			for (uint32 i{ 0 }; i < theta_count; i++) {
				for (uint32 j{ 0 }; j < phi_count; j++) {
					const uint32 index[4]{
						1 + i + j * phi_count,
						1 + i + (j + 1) * phi_count,
						1 + (i + 1) + (j + 1) * phi_count,
						1 + (i + 1) + j * phi_count
					};

					uvs[c] = { j * inv_phi_count, 1.f - (i + 1) * inv_theta_count };
					m.RawIndices[c++] = index[0];
					uvs[c] = { j * inv_phi_count, 1.f - (i + 2) * inv_theta_count };
					m.RawIndices[c++] = index[1];
					uvs[c] = { (j + 1) * inv_phi_count, 1.f - (i + 2) * inv_theta_count };
					m.RawIndices[c++] = index[2];

					uvs[c] = { j * inv_phi_count, 1.f - (i + 1) * inv_theta_count };
					m.RawIndices[c++] = index[0];
					uvs[c] = { (j + 1) * inv_phi_count, 1.f - (i + 2) * inv_theta_count };
					m.RawIndices[c++] = index[2];
					uvs[c] = { (j + 1) * inv_phi_count, 1.f - (i + 1) * inv_theta_count };
					m.RawIndices[c++] = index[3];
				}

				const uint32 index[4]{
					phi_count + i * phi_count,
					phi_count + (i + 1) * phi_count,
					1 + (i + 1) * phi_count,
					1 + i * phi_count
				};

				uvs[c] = { 1.f - inv_phi_count, 1.f - (i + 1) * inv_theta_count };
				m.RawIndices[c++] = index[0];
				uvs[c] = { 1.f - inv_phi_count, 1.f - (i + 2) * inv_theta_count };
				m.RawIndices[c++] = index[1];
				uvs[c] = { 1.f, 1.f - (i + 2) * inv_theta_count };
				m.RawIndices[c++] = index[2];

				uvs[c] = { 1.f - inv_phi_count, 1.f - (i + 1) * inv_theta_count };
				m.RawIndices[c++] = index[0];
				uvs[c] = { 1.f, 1.f - (i + 2) * inv_theta_count };
				m.RawIndices[c++] = index[2];
				uvs[c] = { 1.f , 1.f - (i + 1) * inv_theta_count };
				m.RawIndices[c++] = index[3];
			}

			//Indices for the bottom cap, connecting the south pole to the last ring.
			const uint32 south_pole_index{ (uint32)m.Positions.size() - 1 };
			for (uint32 i{ 0 }; i < (phi_count - 1); i++) {
				uvs[c] = { (2 * i + 1) * 0.5f * inv_phi_count, 0.f };
				m.RawIndices[c++] = south_pole_index;
				uvs[c] = { (i + 1) * inv_phi_count, inv_theta_count };
				m.RawIndices[c++] = south_pole_index - phi_count + i + 1;
				uvs[c] = { 1.f * inv_phi_count, inv_theta_count };
				m.RawIndices[c++] = south_pole_index - phi_count + i;
			}

			uvs[c] = { 1.f - 0.5f * inv_phi_count, 0.f };
			m.RawIndices[c++] = south_pole_index;
			uvs[c] = { 1.f, inv_theta_count };
			m.RawIndices[c++] = south_pole_index - phi_count;
			uvs[c] = { 1.f - inv_phi_count, inv_theta_count };
			m.RawIndices[c++] = south_pole_index - 1;

			assert(c == num_indices);

			m.UVSets.emplace_back(uvs);

			return m;
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
			LODGroup lodGroup;
			lodGroup.Name = "UVSphere";
			lodGroup.Meshes.emplace_back(CreateUVSphere(info));
			scene.LODGroups.emplace_back(lodGroup);
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