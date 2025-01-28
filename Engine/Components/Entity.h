#pragma once
#include "CommonHeaders.h"
#include "ComponentsCommon.h"

namespace Europa {

	#define INIT_INFO(component) namespace component { struct InitInfo; }

	INIT_INFO(TransformComponent);
	INIT_INFO(ScriptComponent)

	#undef INIT_INFO

	//Base class for all the entities in the game engine. And so it begins.
	namespace GameEntity {
		struct EntityInfo {
			TransformComponent::InitInfo* Transform{nullptr};
			ScriptComponent::InitInfo* Script{nullptr};
		};

		//Create a new game entity in the project.
		Entity Create(const EntityInfo& entityInfo);
		//Remove a game entity from the project.
		void Remove(Entity entity);
		//Check, if an Entity is Valid and was not destroyed.
		bool IsAlive(Entity entity);
	}
}