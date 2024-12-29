#pragma once

#include "../Components/ComponentsCommon.h"
#include "TransformComponentAPI.h"

namespace Europa::GameEntity {
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