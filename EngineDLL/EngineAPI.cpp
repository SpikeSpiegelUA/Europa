#ifndef EDITOR_INTERFACE
#define EDITOR_INTERFACE extern "C" __declspec(dllexport)
#endif

#include "CommonHeaders.h"
#include "ID.h"
#include "../Engine/Components/Entity.h"
#include "../Engine/Components/TransformComponent.h"

using namespace Europa;

namespace {
	//Define TransformComponent once more, cause rotation here will have 3 members, not 4.
	struct EditorTransformComponent {
		float position[3];
		float rotation[3];
		float scale[3];

		TransformComponent::InitInfo ConvertToInitInfo() {
			using namespace DirectX;
			TransformComponent::InitInfo initInfo;
			memcpy(&initInfo.position[0], &position[0], sizeof(float) * _countof(position));
			memcpy(&initInfo.scale[0], &scale[0], sizeof(float) * _countof(scale));
			XMFLOAT3A rotation(&rotation[0]);
			XMVECTOR quaternion{ XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3A(&rotation)) };
			XMFLOAT4A rotationQuaternion{};
			XMStoreFloat4A(&rotationQuaternion, quaternion);
			memcpy(&initInfo.rotation[0], &rotationQuaternion.x, sizeof(float) * _countof(initInfo.rotation));
			return initInfo;
		}
	};

	struct GameEntityDescriptor {
		EditorTransformComponent transformComponent;
	};
}

EDITOR_INTERFACE
ID::IDType CreateGameEntity(GameEntityDescriptor* entityDescriptor) {

}