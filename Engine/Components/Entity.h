#pragma once
#include "../Common/CommonHeaders.h"
#include "ComponentsCommon.h"
#include "Script.h"

namespace Europa {

	#define INIT_INFO(component) namespace component { struct InitInfo; }

	INIT_INFO(TransformComponent);
	INIT_INFO(Script)

	#undef INIT_INFO

	//Base class for all the entities in the game engine. And so it begins.
	namespace GameEntity {
		struct EntityInfo {
			TransformComponent::InitInfo* Transform{nullptr};
			Script::InitInfo* Script{nullptr};
		};

		//Create a new game entity in the project.
		Entity Create(const EntityInfo& entityInfo);
		//Remove a game entity from the project.
		void Remove(EntityID entityID);
		//Check, if an Entity is Valid and was not destroyed.
		bool IsAlive(EntityID entityID);
	}
}