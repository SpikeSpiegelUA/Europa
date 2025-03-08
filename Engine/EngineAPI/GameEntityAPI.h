#pragma once

#include "../Components/ComponentsCommon.h"
#include "TransformComponentAPI.h"
#include "ScriptAPI.h"

namespace Europa {
	namespace GameEntity {
		DEFINE_TYPED_ID(EntityID);

		class Entity {
		public:
			constexpr explicit Entity(EntityID id) : ID{ id } {}
			//Creates an entity with an invalid index.
			constexpr Entity() : ID{ ID::InvalidID } {}
			constexpr EntityID GetID() const { return ID; }
			constexpr bool IsValid() const { return ID::IsValid(ID); }

			TransformComponent::Component GetTransform() const;
			Script::Component GetScript() const;
		private:
			EntityID ID;
		};
	}

	namespace Script {
		class EntityScript : public GameEntity::Entity {
		public:
			virtual ~EntityScript() = default;
			virtual void BeginPlay() {};
			virtual void Update(float deltaTime) {};
		protected:
			constexpr explicit EntityScript(GameEntity::Entity entity) : GameEntity::Entity{ entity.GetID()} {};
		};

		namespace Internal {
			using ScriptPointer = std::unique_ptr<EntityScript>;
			using ScriptCreator = ScriptPointer(*)(GameEntity::Entity entity);
			using StringHash = std::hash<std::string>;

			uint8 RegisterScript(size_t, ScriptCreator);
			ScriptCreator GetScriptCreator(size_t tag);

			template<class ScriptClass>
			ScriptPointer CreateScript(GameEntity::Entity entity) {
				assert(entity.IsValid());
				return std::make_unique<ScriptClass>(entity);
			}

#ifdef USE_WITH_EDITOR
			uint8 AddScriptName(const char* name);
#define REGISTER_SCRIPT(TYPE)                                                                                                 \                                                                                                  \
			namespace {                                                                                                       \
				const uint8 Reg##TYPE{ Europa::Script::Internal::RegisterScript                                               \
				(Europa::Script::Internal::StringHash()(#TYPE), &Europa::Script::Internal::CreateScript<TYPE>) };             \
			}																												  \
			const uint8 Name##TYPE {Europa::Script::Internal::AddScriptName(#TYPE)};                                          \
		    }																												  
			#else																											  
			#define REGISTER_SCRIPT(TYPE)                                                                                     \                                                                                                  \
			namespace {                                                                                                       \
				const uint8 Reg##TYPE{ Europa::Script::Internal::RegisterScript                                               \
				(Europa::Script::Internal::StringHash()(#TYPE), &Europa::Script::Internal::CreateScript<TYPE>) };             \
			}
			#endif
		}
	}
}