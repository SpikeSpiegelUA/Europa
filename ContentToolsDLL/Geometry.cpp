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
			const float32 cosAlpha{ XMScalarCos(PI - smoothingAngle * PI / 180.F) };
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
							float32 cos_theta{ 0.f };
							XMVECTOR n2{ XMLoadFloat3(&m.Normals[refs[k]]) };
							if (!isSoftEdge) {
								//We're accounting  for the length of n1 in this calculation because it can possibly change in this loop iteration. 
								//We assume unit length for n2. cos(angle) = dot(n1, n2) / (|n1| |*| |n2|).
								XMStoreFloat(&cos_theta, XMVector3Dot(n1, n2) * XMVector3ReciprocalLength(n1));
							}

							if (isSoftEdge || cos_theta >= cosAlpha) {
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

		void ProcessUVs(Mesh& m) {
			Utilities::Vector<Vertex> oldVertices;
			oldVertices.swap(m.Vertices);
			Utilities::Vector<uint32> oldIndices(m.Indices.size());
			oldIndices.swap(m.Indices);

			const uint32 numVertices{ (uint32)oldVertices.size() };
			const uint32 numIndices{ (uint32)oldIndices.size() };
			assert(numVertices && numIndices);

			Utilities::Vector<Utilities::Vector<uint32>> idxRef(numVertices);
			for (uint32 i{ 0 }; i < numIndices; ++i)
				idxRef[oldIndices[i]].emplace_back(i);

			for (uint32 i{ 0 }; i < numIndices; ++i) {
				auto& refs{ idxRef[i] };
				uint32 numRefs{ (uint32)refs.size() };
				for (uint32 j{ 0 }; j < numRefs; ++j) {
					m.Indices[refs[j]] = (uint32)m.Vertices.size();
					Vertex& v{ oldVertices[oldIndices[refs[j]]] };
					v.UV = m.UVSets[0][refs[j]];
					m.Vertices.emplace_back(v);

					for (uint32 k{ j + 1 }; k < numRefs; ++k) {
						Vector2& uv1{ m.UVSets[0][refs[k]] };
						if (XMScalarNearEqual(v.UV.x, uv1.x, Epsilon) && XMScalarNearEqual(v.UV.y, uv1.y, Epsilon)) {
							m.Indices[refs[k]] = m.Indices[refs[j]];
							refs.erase(refs.begin() + k);
							--numRefs;
							--k;
						}
					}
				}
			}
		}

		void PackVerticesStatic(Mesh& m) {
			const uint32 numVertices{ (uint32)m.Vertices.size() };
			assert(numVertices);
			m.PackedVerticesStatic.reserve(numVertices);

			for (uint32 i{ 0 }; i < numVertices; ++i) {
				Vertex& v{ m.Vertices[i] };
				const uint8 signs{ (uint8)((v.Normal.z > 0.f) << 1) };
				const uint16 normalX{ (uint16)PackFloat<16>(v.Normal.x, -1.f, 1.f) };
				const uint16 normalY{ (uint16)PackFloat<16>(v.Normal.y, -1.f, 1.f) };

				m.PackedVerticesStatic.emplace_back(PackedVertex::VertexStatic{
					v.Position, {0,0,0}, signs, {normalX, normalY}, {}, v.UV
					});
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


		uint64 GetMeshSize(const Mesh& m) {
			const uint64 numVertices{ m.Vertices.size() };
			const uint64 vertexBufferSize{ sizeof(PackedVertex::VertexStatic) * numVertices };
			const uint64 indexSize{ (numVertices < (1 << 16)) ? sizeof(uint16) : sizeof(uint32) };
			const uint64 indexBufferSize{ indexSize * m.Indices.size() };
			constexpr uint64 suint32{ sizeof(uint32) };

			const uint64 size{
				suint32 + m.Name.size() + //A mesh name length and a room for the mesh name string
				suint32 +				  //A mesh id
				suint32 +				  //A vertex size
				suint32 +				  //A number of vertices
				suint32 +				  //An index size(16 or 32 bits)
				suint32 +				  //A number of indices
				sizeof(float32) +		  //A LOD threshold
				vertexBufferSize +		  //A room for vertices
				indexBufferSize			  //A room for indices
			};

			return size;
		}

		uint64 GetSceneSize(const Scene& scene) {
			constexpr uint64 suint32{ sizeof(uint32) };

			uint64 size{
				suint32 +				//Name length
				scene.Name.size() +     //Room for scene name string
				suint32                 //Number of LODs
			};

			for (auto& lod : scene.LODGroups) {
				uint64 lodSize{
					suint32 + lod.Name.size() +  //LOD name length and room for LPD name string
					suint32                      //Number of meshes in this LOD
				};

				for (auto& m : lod.Meshes) {
					lodSize += GetMeshSize(m);
				}

				size += lodSize;
			}

			return size;
		}

		void PackMeshData(const Mesh& mesh, uint8* const buffer, uint64& at) {
			constexpr uint64 suint32{ sizeof(uint32) };
			uint32 source{ 0 };
			//Mesh name
			source = (uint32)mesh.Name.size();
			memcpy(&buffer[at], &source, suint32);
			at += suint32;
			memcpy(&buffer[at], mesh.Name.c_str(), source);
			at += source;
			//LOD ID
			source = mesh.LODID;
			memcpy(&buffer[at], &source, suint32);
			at += suint32;
			//Vertex size
			constexpr uint32 vertexSize{ sizeof(PackedVertex::VertexStatic) };
			source = vertexSize;
			memcpy(&buffer[at], &source, suint32);
			at += suint32;
			//Number of vertices
			const uint32 numVertices{ (uint32)mesh.Vertices.size() };
			source = numVertices;
			memcpy(&buffer[at], &source, suint32);
			at += suint32;
			//Index size
			const uint32 indexSize{ (numVertices < (1 << 16)) ? sizeof(uint16) : sizeof(uint32) };
			source = indexSize;
			memcpy(&buffer[at], &source, suint32);
			at += suint32;
			//Number of indices
			const uint32 numIndices{ (uint32)mesh.Indices.size() };
			source = numIndices;
			memcpy(&buffer[at], &source, suint32);
			at += suint32;
			//LOD threshold
			memcpy(&buffer[at], &mesh.LODThreshold, sizeof(float32));
			at += sizeof(float32);
			//Vertex data.
			source = vertexSize * numVertices;
			memcpy(&buffer[at], mesh.PackedVerticesStatic.data(), source);
			at += source;
			//Index data
			source = indexSize * numIndices;
			void* data{ (void*)mesh.Indices.data() };
			Utilities::Vector<uint16> indices;

			if (indexSize == sizeof(uint16)) {
				indices.resize(numIndices);
				for (uint32 i = 0; i < numIndices; ++i)
					indices[i] = (uint16)mesh.Indices[i];
				data = (void*)indices.data();
			}

			memcpy(&buffer[at], data, source);
			at += source;
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
		constexpr uint64 suint32{ sizeof(uint32) };
		const uint64 sceneSize{ GetSceneSize(scene) };
		data.BufferSize = (uint32)sceneSize;
		data.Buffer = (uint8*)CoTaskMemAlloc(sceneSize);
		assert(data.Buffer);

		uint8* const buffer{ data.Buffer };
		uint64 at{ 0 };
		uint32 source{ 0 };

		//Scene name.
		source = (uint32)scene.Name.size();
		memcpy(&buffer[at], &source, suint32);
		at += suint32;
		memcpy(&buffer[at], scene.Name.c_str(), source);
		at += source;

		//Number of LODs
		source = (uint32)scene.LODGroups.size();
		memcpy(&buffer[at], &source, suint32);
		at += suint32;

		for (auto& lod : scene.LODGroups) {
			//LOD name
			source = (uint32)lod.Name.size();
			memcpy(&buffer[at], &source, suint32);
			at += suint32;
			memcpy(&buffer[at], lod.Name.c_str(), source);
			at += source;
			//Number of meshes in this LOD
			source = (uint32)lod.Meshes.size();
			memcpy(&buffer[at], &source, suint32);
			at += suint32;

			for (auto& mesh : lod.Meshes) {
				PackMeshData(mesh, buffer, at);
			}
		}

		assert(sceneSize == at);
	}

}