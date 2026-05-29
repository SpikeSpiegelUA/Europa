#include "ContentLoader.h"
#include "../Components/Entity.h"
#include "../Components/TransformComponent.h"
#include "../Components/Script.h"
#include "Graphics/Renderer.h"

#if !defined(SHIPPING)

#include <fstream>
#include <filesystem>
#include <Windows.h>

namespace Europa::Content {
	namespace {
		enum EComponentType {
			Transform,
			Script,
			Count
		};

		Utilities::Vector<GameEntity::Entity> entities;
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
			char scriptName[256]{};
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

		bool ReadFile(std::filesystem::path path, std::unique_ptr<uint8[]>& data, uint64& size) 
		{
			if (!std::filesystem::exists(path))
				return false;

			size = std::filesystem::file_size(path);
			assert(size);
			if (!size)
				return false;
			data = std::make_unique<uint8[]>(size);
			std::ifstream file{ path, std::ios::in | std::ios::binary };
			if (!file || !file.read((char*)data.get(), size)) 
			{
				file.close();
				return false;
			}

			file.close();
			return true;
		}
	}//Anonymous namespace.
	bool LoadGame()
	{
		//Read game.bin file and create the entities from it.
		std::unique_ptr<uint8[]> gameData{};
		uint64 size{ 0 };
		if (!ReadFile("game.bin", gameData, size))
			return false;
		assert(gameData.get());
		const uint8* at{ gameData.get() };
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
			entities.EmplaceBack(entity);
		}

		assert(at == gameData.get() + size);
		return true;
	}

	void UnloadGame()
	{
		for (auto entity : entities)
			GameEntity::Remove(entity.GetID());
	}

	bool LoadEngineShaders(std::unique_ptr<uint8[]>& shaders, uint64& size) 
	{
		auto path = Graphics::GetEngineShadersPath();
		return ReadFile(path, shaders, size);
	}
#endif //!defined(SHIPPING)
}
