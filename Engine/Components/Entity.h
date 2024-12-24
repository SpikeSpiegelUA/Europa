#pragma once

#include "CommonHeaders.h"
#include "ComponentsCommon.h"

namespace Europa {

	#define INIT_INFO(component) namespace component { struct InitInfo; }

	INIT_INFO(TransformComponent);

	#undef INIT_INFO

	//Base class for all the entities in the game engine. And so it begins.
	namespace GameEntity {
		struct EntityInfo {
			TransformComponent::InitInfo* Transform{};
		};

		//Create a new game entity in the project.
		EntityID CreateGameEntity(const EntityInfo& entityInfo);
		//Remove a game entity from the project.
		void RemoveGameEntity(EntityID id);
		//Check, if an Entity is Valid and was not destroyed.
		bool IsAlive(EntityID id);
	}
}