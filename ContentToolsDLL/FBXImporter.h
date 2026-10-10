#pragma once
#include "ToolsCommon.h"
#include <fbxsdk.h>

namespace Europa::Tools {
	struct SceneData;
	struct Scene;
	struct Mesh;
	struct GeometryImportSettings;

	class FBXContext {
	public:
		FBXContext(const char* file, Scene* scene, SceneData* data)
			:scene{ scene }, sceneData{ data }
		{
			assert(file && scene && sceneData);
			if (InitializeFBX()) 
			{
				LoadFBXFile(file);
				assert(IsValid());
			}
		}

		~FBXContext()
		{
			fbxScene->Destroy();
			fbxManager->Destroy();
			ZeroMemory(this, sizeof(FBXContext));
		}

		void GetScene(FbxNode* root = nullptr);

		constexpr bool IsValid() const {
			return fbxManager && fbxScene;
		}
		constexpr float32 SceneScale() const {
			return sceneScale;
		}
	private:

		bool InitializeFBX();
		void LoadFBXFile(const char* file);
		void GetMesh(FbxNode* node, Utilities::Vector<Mesh>& meshes);
		void GetLODGroup(FbxNode* node);

		Scene* scene{ nullptr };
		SceneData* sceneData{ nullptr };
		FbxManager* fbxManager{ nullptr };
		FbxScene* fbxScene{ nullptr };
		float32 sceneScale{ 1.f };
	};
}
