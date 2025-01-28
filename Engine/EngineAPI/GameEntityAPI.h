#pragma once

#include "../Components/ComponentsCommon.h"
#include "TransformComponentAPI.h"
#include "ScriptComponentAPI.h"
#include <memory>

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

			template<class ScriptClass>
			ScriptPointer CreateScript(GameEntity::Entity entity) {
				assert(entity.IsValid());
				return std::make_unique<ScriptClass>(entity);
			}
		}
	}
}