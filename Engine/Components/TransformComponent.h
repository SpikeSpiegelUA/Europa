#pragma once

#include "ComponentsCommon.h"

namespace Europa::TransformComponent {
	DEFINE_TYPED_ID(TransformID);

	struct InitInfo {
		float position[3]{};
		float rotation[4]{};
		float scale[3]{1.f, 1.f, 1.f};
	};

	TransformID CreateTransform(const InitInfo& initInfo, GameEntity::EntityID entityID);
	void RemoveTransformComponent()
}