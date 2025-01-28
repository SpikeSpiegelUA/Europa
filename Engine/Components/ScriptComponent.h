#pragma once

#include "ComponentsCommon.h"
#include "../EngineAPI/ScriptComponentAPI.h"

namespace Europa::ScriptComponent {

	struct InitInfo {
		float position[3]{ 0.f, 0.f, 0.f };
		float rotation[4]{ 0.f, 0.f, 0.f, 0.f };
		float scale[3]{ 1.f, 1.f, 1.f };
	};

	Component Create(const InitInfo& initInfo, GameEntity::Entity entity);
	void Remove(Component componentToRemove);
}