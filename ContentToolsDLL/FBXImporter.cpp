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

//LNK4099 PDB not found warnings can be resolved either by installing FBX SDK PDBs (separate download) or
//by disabling this warning in linked options(linked command line: /ignore:4099).

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

			LODGroup lod{};
			GetMeshes(node, lod.Meshes, 0, -1.f);
			if (lod.Meshes.Size()) 
			{
				lod.Name = lod.Meshes[0].Name;
				scene->LODGroups.EmplaceBack(lod);
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

	void FBXContext::GetMeshes(FbxNode* node, Utilities::Vector<Mesh>& meshes, uint32 lodID, float32 lodThreshold)
	{
		assert(node && lodID != uint32_invalid_id);
		bool isLODGroup{ false };

		if (const int32 numAttributes{ node->GetNodeAttributeCount() })
		{
			for (int32 i{ 0 }; i < numAttributes; i++) {
				FbxNodeAttribute* attribute{ node->GetNodeAttributeByIndex(i) };
				const FbxNodeAttribute::EType attributeType{ attribute->GetAttributeType() };
				if (attributeType == FbxNodeAttribute::eMesh) {
					GetMesh(attribute, meshes, lodID, lodThreshold);
				}
				else if (attributeType == FbxNodeAttribute::eLODGroup) {
					GetLODGroup(attribute);
					isLODGroup = true;
				}
			}
		}

		if (!isLODGroup) {
			if (const int32 numChildren{ node->GetChildCount() }) {
				for (int32 i{ 0 }; i < numChildren; i++) {
					GetMeshes(node->GetChild(i), meshes, lodID, lodThreshold);
				}
			}
		}
	}

	void FBXContext::GetMesh(FbxNodeAttribute* attribute, Utilities::Vector<Mesh>& meshes, uint32 lodID, float32 lodThreshold)
	{
		assert(attribute);

		FbxMesh* fbxMesh{ (FbxMesh*)attribute };
		if (fbxMesh->RemoveBadPolygons() < 0)
			return;

		//Triangulate the mesh if needed.
		FbxGeometryConverter gc{ fbxManager };
		fbxMesh = static_cast<FbxMesh*>(gc.Triangulate(fbxMesh, true));
		if (!fbxMesh || fbxMesh->RemoveBadPolygons() < 0)
			return;

		FbxNode* const node{ fbxMesh->GetNode() };

		Mesh m;
		m.LODID = lodID;
		m.LODThreshold = lodThreshold;
		m.Name = (node->GetName()[0] != '\0') ? node->GetName() : fbxMesh->GetName();

		if (GetMeshData(fbxMesh, m)) 
		{
			meshes.EmplaceBack(m);
		}

		//See if there's a mesh somewhere further down the hierarchy
		GetScene(node);
	}

	void FBXContext::GetLODGroup(FbxNodeAttribute* attribute)
	{
		assert(attribute);

		FbxLODGroup* lodGroup{ (FbxLODGroup*)attribute };
		FbxNode* const node{ lodGroup->GetNode() };
		LODGroup lod{};
		lod.Name = (node->GetName()[0] != '\0') ? node->GetName() : lodGroup->GetName();
		//Note: number of LODs is exclusive the base mesh(LOD 0)
		const int32 numNodes{ node->GetChildCount() };
		assert(numNodes > 0 && lodGroup->GetNumThresholds() == (numNodes - 1));

		for (int32 i{ 0 }; i < numNodes; i++) 
		{
			float32 lodThreshold{ -1.f};
			if (i > 0) {
				FbxDistance threshold;
				lodGroup->GetThreshold(i - 1, threshold);
				lodThreshold = threshold.value() * sceneScale;
			}

			GetMeshes(node->GetChild(i), lod.Meshes, (uint32)lod.Meshes.Size(), lodThreshold);
		}

		if (lod.Meshes.Size())
			scene->LODGroups.EmplaceBack(lod);
	}


	EDITOR_INTERFACE void ImportFbx(const char* file, SceneData* data) {
		assert(file && data);
		Scene scene{};

		//NOTE: anything that involves using the FBX SDK should be single-threaded.
		{
			std::lock_guard lock{ fbxMutex };
			FBXContext fbxContext{ file, &scene, data };
			if (fbxContext.IsValid()) {
				fbxContext.GetScene();
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

		FbxNode* const node{ fbxMesh->GetNode() };
		FbxAMatrix geometricTransform;

		geometricTransform.SetT(node->GetGeometricTranslation(FbxNode::eSourcePivot));
		geometricTransform.SetR(node->GetGeometricRotation(FbxNode::eSourcePivot));
		geometricTransform.SetS(node->GetGeometricScaling(FbxNode::eSourcePivot));

		FbxAMatrix transform{ node->EvaluateGlobalTransform() * geometricTransform };
		FbxAMatrix inverseTranspose{ transform.Inverse().Transpose() };

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
				const uint32 vIdx{(uint32)indices[i]};
				//Did we encounter this vertex before? If so, just add its index.
				//If not, add the vertex and a new index.
				if(vertexRef[vIdx] != uint32_invalid_id)
				{
					m.RawIndices[i] = vertexRef[vIdx];
				}
				else
				{
					FbxVector4 v = transform.MultT(vertices[vIdx]) * sceneScale;
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
					FbxVector4 n{ inverseTranspose.MultT(normals[i]) };
					n.Normalize();
					m.Normals.EmplaceBack((float32)n[0], (float32)n[1], (float32)n[2]);
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
					//TODO: not sure if this transformation is correct.
					FbxVector4 t{ tangents->GetAt(i) };
					const float32 handedness{ (float32)t[3] };
					t[3] = 0.0;
					t.Normalize();
					t = inverseTranspose.MultT(t);
					m.Tangents.EmplaceBack((float32)t[0], (float32)t[1], (float32)t[2], handedness);
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