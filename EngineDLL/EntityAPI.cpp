
#pragma comment (lib, "Engine.lib")

#include "Common.h"
#include "CommonHeaders.h"
#include "ID.h"
#include "../Engine/Components/Entity.h"
#include "../Engine/Components/TransformComponent.h"
#include "../Engine/Components/Script.h"

using namespace Europa;

namespace {
	//Define TransformComponent once more, cause rotation of an editor's game entity will have 3 members, not 4.
	struct APITransformComponent {
		float position[3];
		float rotation[3];
		float scale[3];

		TransformComponent::InitInfo ConvertToInitInfo() {
			using namespace DirectX;
			TransformComponent::InitInfo initInfo;
			memcpy(&initInfo.position[0], &position[0], sizeof(float) * _countof(position));
			memcpy(&initInfo.scale[0], &scale[0], sizeof(float) * _countof(scale));
			XMFLOAT3A xmRotation(&this->rotation[0]);
			XMVECTOR quaternion{ XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3A(&xmRotation)) };
			XMFLOAT4A rotationQuaternion{};
			XMStoreFloat4A(&rotationQuaternion, quaternion);
			memcpy(&initInfo.rotation[0], &rotationQuaternion.x, sizeof(float) * _countof(initInfo.rotation));
			return initInfo;
		}
	};

	struct APIScriptComponent {
		Script::Internal::ScriptCreator scriptCreator;

		Script::InitInfo ConvertToInitInfo() {
			Script::InitInfo info{};
			info.ScriptCreator = scriptCreator;
			return info;
		}
	};

	struct GameEntityDescriptor {
		APITransformComponent editorTransformComponent;
		APIScriptComponent editorScriptComponent;
	};

	GameEntity::Entity CreateGameEntityFromID(ID::IDType id) {
		return GameEntity::Entity{ GameEntity::EntityID{id} };
	}
} //Anonymous namespace, to keep contained members only in this translation unit.

EDITOR_INTERFACE
ID::IDType CreateGameEntity(GameEntityDescriptor* entityDescriptor) {
	assert(entityDescriptor);
	GameEntityDescriptor& descriptor{ *entityDescriptor };
	TransformComponent::InitInfo transformComponentInitInfo{ descriptor.editorTransformComponent.ConvertToInitInfo() };
	Script::InitInfo scriptComponentInitInfo{ descriptor.editorScriptComponent.ConvertToInitInfo() };
	GameEntity::EntityInfo gameEntityInfo{
		&transformComponentInitInfo,
		&scriptComponentInitInfo
	};
	return GameEntity::Create(gameEntityInfo).GetID();
}

EDITOR_INTERFACE
void RemoveGameEntity(ID::IDType id) {
	assert(ID::IsValid(id));
	GameEntity::Remove(GameEntity::EntityID{ id });
}