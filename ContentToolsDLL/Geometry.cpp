#include "Geometry.h"

namespace Europa::Tools {
	namespace {

		using namespace Math;
		using namespace DirectX;

		void RecalculateNormals(Mesh& m) {
			const uint32 numIndices{ (uint32)m.RawIndices.size() };
			m.Normals.reserve(numIndices);

			for (uint32 i{ 0 }; i < numIndices; ++i) {
				const uint32 i0{ m.RawIndices[i] };
				const uint32 i1{ m.RawIndices[++i] };
				const uint32 i2{ m.RawIndices[++i] };

				XMVECTOR v0{ XMLoadFloat3(&m.Positions[i0]) };
				XMVECTOR v1{ XMLoadFloat3(&m.Positions[i1]) };
				XMVECTOR v2{ XMLoadFloat3(&m.Positions[i2]) };

				XMVECTOR e0{ v1 - v0 };
				XMVECTOR e1{ v2 - v0 };
				XMVECTOR n{ XMVector3Normalize(XMVector3Cross(e0, e1)) };

				XMStoreFloat3(&m.Normals[i], n);
				m.Normals[i - 1] = m.Normals[i];
				m.Normals[i - 2] = m.Normals[i];
			}
		}

		void ProcessNormals(Mesh& m, float32 smoothingAngle) {
			const float32 cosAngle{ XMScalarCos(PI - smoothingAngle * PI / 180.F) };
			const bool isHardEdge{ XMScalarNearEqual(smoothingAngle, 180.f, Epsilon) };
			const bool isSoftEdge{ XMScalarNearEqual(smoothingAngle, 0.f, Epsilon) };
			const uint32 numIndices{ (uint32)m.RawIndices.size() };
			const uint32 numVertices{ (uint32)m.Positions.size() };
			assert(numIndices && numVertices);
			
			m.Indices.resize(numIndices);

			Utilities::Vector<Utilities::Vector<uint32>> idxRef(numVertices);
			for(uint32 i{0}; i < numIndices; ++i)
				idxRef[m.RawIndices[i]].emplace_back(i);



			for (uint32 i{ 0 }; i < numVertices; ++i) {
				auto& refs{ idxRef[i] };
				uint32 numRefs{ (uint32)refs.size() };
				for (uint32 j{ 0 }; j < numRefs; ++j) {
					m.Indices[refs[j]] = (uint32)m.Vertices.size();
					Vertex& v{ m.Vertices.emplace_back() };
					v.Position = m.Positions[m.RawIndices[refs[j]]];

					XMVECTOR n1{ XMLoadFloat3(&m.Normals[refs[j]]) };
					if (!isHardEdge) {
						for (uint32 k{ j + 1 }; k < numRefs; ++k)
						{
							float32 n{ 0.f };
							XMVECTOR n2{ XMLoadFloat3(&m.Normals[refs[k]]) };
							if (!isSoftEdge) {
								//We're accounting  for the length of n1 in this calculation because it can possibly change in this loop iteration. 
								//We assume unit length for n2. cos(angle) = dot(n1, n2) / (|n1| |*| |n2|).
								XMStoreFloat(&n, XMVector3Dot(n1, n2) * XMVector3ReciprocalLength(n1));
							}

							if (isSoftEdge || n >= cosAngle) {
								n1 += n2;
								m.Indices[refs[k]] = m.Indices[refs[j]];
								refs.erase(refs.begin() + k);
								--numRefs;
								--k;
							}
						}
					}
					XMStoreFloat3(&v.Normal, XMVector3Normalize(n1));
				}
			}
		}

		void ProcessVertices(Mesh& m, const GeometryImportSettings& settings) {
			assert((m.RawIndices.size() % 3) == 0);
			if (settings.CalculateNormals || m.Normals.empty()) {
				RecalculateNormals(m);
			}

			ProcessNormals(m, settings.SmoothingAngle);

			if (!m.UVSets.empty()) {
				ProcessUVs(m);
			}

			PackVerticesStatic(m);
		}

	} //anonymous namespace

	void ProcessScene(Scene& scene, const GeometryImportSettings& settings)
	{
		for(auto& lod: scene.LODGroups)
			for (auto& m : lod.Meshes) {
				ProcessVertices(m, settings);
			}
	}

	void PackData(const Scene& scene, SceneData& data)
	{

	}

}