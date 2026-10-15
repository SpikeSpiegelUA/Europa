#include "FBXImporter.h"
#include "Geometry.h"

//If you get any compilation or linker errors than make sure that
//1) FBX SDK 2020.2 or later is installed on your system
//2) The include path to fbxsdk.h is added to "Additional Include Directories" (compiler settings)
//3)The library paths in the following section point to the correct location
#if _DEBUG
#pragma comment (lib, "C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.11\\lib\\x64\\debug\\libfbxsdk-md.lib")
#pragma comment (lib, "C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.11\\lib\\x64\\debug\\libxml2-md.lib")
#pragma comment (lib, "C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.11\\lib\\x64\\debug\\zlib-md.lib")
#else
#pragma comment (lib, "C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.11\\lib\\x64\\release\\libfbxsdk-md.lib")
#pragma comment (lib, "C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.11\\lib\\x64\\release\\libxml2-md.lib")
#pragma comment (lib, "C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.11\\lib\\x64\\release\\zlib-md.lib")
#endif

namespace Europa::Tools {
	namespace {
		std::mutex fbxMutex;
	} //Anonymous namespace

	void FBXContext::GetScene(FbxNode* root /*= nullptr*/)
	{
		assert(IsValid());

		if (!root) 
		{
			root = fbxScene->GetRootNode();
			if (!root)
				return;
		}

		const int32 numNodes{ root->GetChildCount() };
		for (int32 i{ 0 }; i < numNodes; ++i) 
		{
			FbxNode* node{ root->GetChild(i) };
			if (!node)
				continue;

			if (node->GetMesh())
			{
				LODGroup lod{};
				GetMesh(node, lod.Meshes);
				if (lod.Meshes.Size()) 
				{
					lod.Name = lod.Meshes[0].Name;
					scene->LODGroups.EmplaceBack(lod);
				}
			}
			else if (node->GetLodGroup())
			{
				GetLODGroup(node);
			}
			else {
				//See if there's a mesh somewhere further down the hierarchy.
				GetScene(node);
			}
		}
	}

	bool FBXContext::InitializeFBX()
	{
		assert(!IsValid());

		fbxManager = FbxManager::Create();
		if (!fbxManager) 
		{
			return false;
		}

		FbxIOSettings* ios{ FbxIOSettings::Create(fbxManager, IOSROOT) };
		assert(ios);
		fbxManager->SetIOSettings(ios);

		return true;
	}
	
	void FBXContext::LoadFBXFile(const char* file) 
	{
		assert(fbxManager && !fbxScene);
		fbxScene = FbxScene::Create(fbxManager, "Importer Scene");
		if (!fbxScene) {
			return;
		}

		FbxImporter* importer{ FbxImporter::Create(fbxManager, "Importer") };
		if (!(importer && importer->Initialize(file, -1, fbxManager->GetIOSettings()) &&
			importer->Import(fbxScene))) 
		{
			return;
		}

		importer->Destroy();

		//Get scene scale in meters.
		sceneScale = (float32)fbxScene->GetGlobalSettings().GetSystemUnit().GetConversionFactorTo(FbxSystemUnit::m);
	}

	void FBXContext::GetMesh(FbxNode* node, Utilities::Vector<Mesh>& meshes)
	{
		assert(node);

		if (FbxMesh * fbxMesh{ node->GetMesh() }) {
			if (fbxMesh->RemoveBadPolygons() < 0)
				return;

			//Triangulate the mesh if needed.
			FbxGeometryConverter gc{ fbxManager };
			fbxMesh = static_cast<FbxMesh*>(gc.Triangulate(fbxMesh, true));
			if (!fbxMesh || fbxMesh->RemoveBadPolygons() < 0)
				return;

			Mesh m;
			m.LODID = (uint32)meshes.Size();
			m.LODThreshold = -1.f; 
			m.Name = (node->GetName()[0] != '\0') ? node->GetName() : fbxMesh->GetName();

			if (GetMeshData(fbxMesh, m)) 
			{
				meshes.EmplaceBack(m);
			}
		}
	}

	void FBXContext::GetLODGroup(FbxNode * node)
	{
		assert(node);

		if (FbxLODGroup * lodGroup{ node->GetLodGroup() }) 
		{
			LODGroup lod{};
			lod.Name = (node->GetName()[0] != '\0') ? node->GetName() : lodGroup->GetName();
			//Note: number of LODs is exclusive the base mesh(LOD 0)
			const int32 numLods{ lodGroup->GetNumThresholds() };
			const int32 numNodes{ node->GetChildCount() };
			assert(numLods > 0 && numNodes > 0);

			for (int32 i{ 0 }; i < numNodes; i++) 
			{
				GetMesh(node->GetChild(i), lod.Meshes);

				if (lod.Meshes.Size() > 1 && lod.Meshes.Size() <= numLods + 1 && lod.Meshes.Back().LODThreshold < 0.f)
				{
					FbxDistance threshold;
					lodGroup->GetThreshold((uint32)lod.Meshes.Size() - 2, threshold);
					lod.Meshes.Back().LODThreshold = threshold.Value() * sceneScale;
				}

				if (lod.Meshes.Size())
					scene->LODGroups.EmplaceBack(lod);
			}
		}
	}


	EDITOR_INTERFACE void ImportFbx(const char* file, SceneData* data) {
		assert(file && data);
		Scene scene{};

		//NOTE: anything that involves using the FBX SDK should be single-threaded.
		{
			std::lock_guard lock{ fbxMutex };
			FBXContext fbxContext{ file, &scene, data };
			if (fbxContext.IsValid()) {
				//get scene.
			}
			else {
				//TODO: send failure log message to editor.
				return;
			}
		}

		ProcessScene(scene, data->Settings);
		PackData(scene, *data);
	}

	bool FBXContext::GetMeshData(FbxMesh* fbxMesh, Mesh& m)
	{
		assert(fbxMesh);

		const int32 numPolys{fbxMesh->GetPolygonCount()};
		if(numPolys <= 0)
			return false;

		//Get Vertices.
		const int32 numVertices{fbxMesh->GetControlPointsCount()};
		FbxVector4* vertices{fbxMesh->GetControlPoints()};
		const int32 numIndices{fbxMesh->GetPolygonVertexCount()};
		int32* indices{fbxMesh->GetPolygonVertices()};

		assert(numVertices > 0 && vertices && numIndices > 0 && indices);
		if(!(numVertices > 0 && vertices && numIndices > 0 && indices))
			return false;

			m.RawIndices.Resize(numIndices);
			Utilities::Vector vertexRef(numVertices, uint32_invalid_id);

			for(int32 i{0}; i< numIndices; i++)
			{
				const uint32 vIdx{indices[i]};
				//Did we encounter this vertex before? If so, just add its index.
				//If not, add the vertex and a new index.
				if(vertexRef[vIdx] != uint32_invalid_id)
				{
					m.RawIndices[i] = vertexRef[vIdx];
				}
				else
				{
					FbxVector4 v = vertices[vIdx] * sceneScale;
					m.RawIndices[i] = (uint32)m.Positions.Size();
					vertexRef[vIdx] = m.RawIndices[i];
					m.Positions.EmplaceBack((float32)v[0], (float32)v[1], (float32)v[2]);
				}
			}

		assert(m.RawIndices.Size() % 3 == 0);

		//Get material index per polygon.
		assert(numPolys > 0);
		FbxLayerElementArrayTemplate<int32>* mtlIndices;
		if(fbxMesh->GetMaterialIndices(&mtlIndices))
		{
			for(int32 i{0}; i < numPolys; i++){
				const int32 mtlIndex{mtlIndices->GetAt(i)};
				assert(mtlIndex >= 0);
				m.MaterialIndices.EmplaceBack((uint32)mtlIndex);
				if(std::find(m.MaterialUsed.Begin(), m.MaterialUsed.End(), (uint32)mtlIndex) == m.MaterialUsed.End())
				{
					m.MaterialUsed.EmplaceBack((uint32)mtlIndex);
				}
			}
		}

		//Importing normals is ON by default
		const bool importNormals{!sceneData->Settings.CalculateNormals};
		//Importing tangents is OFF by default.
		const bool importTangents{ !sceneData->Settings.CalculateTangents };

		//Import normals
		if (importNormals) {
			FbxArray<FbxVector4> normals;
			//Calculate normals using FBX's built-in method, but only if no normal data is already there.
			if (fbxMesh->GenerateNormals() && fbxMesh->GetPolygonVertexNormals(normals) && normals.Size() > 0) 
			{
				const int32 numNormals{ normals.Size() };
				for (int32 i{ 0 }; i < numNormals; i++) 
				{
					m.Normals.EmplaceBack((float32)normals[i][0], (float32)normals[i][1], (float32)normals[i][2]);
				}
			}
			else
			{
				//Something went wrong with importing normals from FBX.
				//Fall back to our own normal calculation method.
				sceneData->Settings.CalculateNormals = true;
			}
		}

		//Import tangents.
		if (importTangents) 
		{
			FbxLayerElementArrayTemplate<FbxVector4>* tangents{ nullptr };
			//Calculate tangents using FBX's built-in method, but only if no tangent data is already there.
			if (fbxMesh->GenerateTangentsData() && fbxMesh->GetTangents(&tangents) && tangents && tangents->GetCount() > 0) 
			{
				const int32 numTangent{ tangents->GetCount() };
				for (int32 i{ 0 }; i < numTangent; i++) 
				{
					FbxVector4 t{ tangents->GetAt(i) };
					m.Tangents.EmplaceBack((float32)t[0], (float32)t[1], (float32)t[2], (float32)t[3]);
				}
			}
			else 
			{
				//Something went wrong with importing tangents from FBX.
				//Fall back to our own tangent calculation method.
				sceneData->Settings.CalculateTangents = true;
			}
		}

		FbxStringList uvNames;
		fbxMesh->GetUVSetNames(uvNames);
		const int32 uvSetCount{ uvNames.GetCount() };
		//NOTE: it's ok if we don't have a uv set. Fore example, some emissive objects don't need a uv map.
		m.UVSets.Resize(uvSetCount);

		for (int32 i{ 0 }; i < uvSetCount; i++) 
		{
			FbxArray<FbxVector2> uvs;
			if (fbxMesh->GetPolygonVertexUVs(uvNames.GetStringAt(i), uvs)) 
			{
				const int32 numUVs{ uvs.Size() };
				for (int32 j{ 0 }; j < numUVs; j++) 
				{
					m.UVSets[i].EmplaceBack((float32)uvs[j][0], (float32)uvs[j][1]);
				}
			}
		}

		return true;
	}
}