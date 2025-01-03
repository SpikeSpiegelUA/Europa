#pragma once

#include "ComponentsCommon.h"

namespace Europa::TransformComponent {

	struct InitInfo {
		float position[3]{0.f, 0.f, 0.f};
		float rotation[4]{0.f, 0.f, 0.f, 0.f};
		float scale[3]{1.f, 1.f, 1.f};
	};

	Component CreateTransform(const InitInfo& initInfo, GameEntity::Entity entity);
	void RemoveTransformComponent(Component componentToRemove);
}