#include "FBXImporter.h"
#include "Geometry.h"

namespace Europa::Tools {
	namespace {
		std::mutex fbxMutex;
	} //Anonymous namespace

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
			}
		}

		ProcessScene(scene, data->Settings);
		PackData(scene, *data);
	}
}