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

		const bool importNormals{!sceneData->Settings.CalculateNormals};
	}
}