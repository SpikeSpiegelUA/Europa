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
}