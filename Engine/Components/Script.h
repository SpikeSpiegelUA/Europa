#pragma once

#include "ComponentsCommon.h"
#include "../EngineAPI/ScriptAPI.h"

namespace Europa::Script {

	struct InitInfo {
		Script::Internal::ScriptCreator ScriptCreator;
	};

	Component Create(const InitInfo& initInfo, GameEntity::Entity entity);
	void Remove(Component componentToRemove);
}