#include "ContentLoader.h"
#include "../Components/Entity.h"
#include "../Components/TransformComponent.h"
#include "../Components/ScriptComponent.h"
#include <fstream>

#if !defined(SHIPPING)
namespace Europa::Content {
	namespace {
		enum EComponentType {
			Transform,
			Script,
			Count
		};

		Utilities::vector<GameEntity::Entity> entities;
		TransformComponent::InitInfo TransformInfo{};
		Script::InitInfo ScriptInfo{};

		bool ReadTransform(const uint8*& data, GameEntity::EntityInfo& info) {
			using namespace DirectX;
			float rotation[3]{};
			
			assert(!info.Transform);
			memcpy(&TransformInfo.position[0], data, sizeof(TransformInfo.position));
			data += sizeof(TransformInfo.position);
			memcpy(&rotation[0], data, sizeof(rotation));
			data += sizeof(rotation);
			memcpy(&TransformInfo.scale[0], data, sizeof(TransformInfo.scale));
			data += sizeof(TransformInfo.scale);

			XMFLOAT3A rot{ &rotation[0] };
			XMVECTOR quat{ XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3A(&rot)) };
			XMFLOAT4A rotQuaternion{};
			XMStoreFloat4A(&rotQuaternion, quat);
			memcpy(&TransformInfo.rotation[0], &rotQuaternion.x, sizeof(TransformInfo.rotation));
			info.Transform = &TransformInfo;
			return true;
		}

		bool ReadScript(const uint8*& data, GameEntity::EntityInfo& info) {
			assert(!info.Script);
			const uint32 nameLength{ *data };
			data += sizeof(uint32);
			assert(nameLength < 256);
			char scriptName[256];
			memcpy(&scriptName[0], data, nameLength);
			data += nameLength;
			scriptName[nameLength] = 0;
			ScriptInfo.ScriptCreator = Script::Internal::GetScriptCreator(Script::Internal::StringHash()(scriptName));
			info.Script = &ScriptInfo;
			return ScriptInfo.ScriptCreator != nullptr;
		}

		using ComponentReader = bool(*)(const uint8*&, GameEntity::EntityInfo&);
		ComponentReader ComponentReaders[]{
			ReadTransform,
			ReadScript
		};
		static_assert(_countof(ComponentReaders) == EComponentType::Count);
	}
	bool LoadGame()
	{
		std::ifstream game("game.bin", std::ios::in | std::ios::binary);
		Europa::Utilities::vector<uint8> buffer(std::istreambuf_iterator<char>(game), {});
		assert(buffer.size());
		const uint8* at{ buffer.data() };
		constexpr uint32 suint32{ sizeof(uint32) };
		const uint32 numEntities{ *at };
		at += suint32;
		if (!numEntities)
			return false;

		for (uint32 entityIndex{ 0 }; entityIndex < numEntities; ++entityIndex) {
			GameEntity::EntityInfo info{};
			const uint32 entityType{ *at };
			at += suint32;
			const uint32 numComponents{ *at };
			at += suint32;
			if (!numComponents)
				return false;

			for (uint32 componentIndex{ 0 }; componentIndex < numComponents; ++componentIndex) {
				const uint32 componentType{ *at };
				at += suint32;
				assert(componentType < EComponentType::Count);
				if (!ComponentReaders[componentType](at, info))
					return false;
			}

			assert(info.Transform);
			GameEntity::Entity entity{ GameEntity::Create(info) };
			if (!entity.IsValid())
				return false;
			entities.emplace_back(entity);
		}

		assert(at == buffer.data() + buffer.size());
		return true;
	}

	void UnloadGame()
	{
		for (auto entity : entities)
			GameEntity::Remove(entity.GetID());
	}
#endif
}
