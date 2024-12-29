#pragma once
#include "../Components/ComponentsCommon.h"

namespace Europa::TransformComponent {
	DEFINE_TYPED_ID(TransformID);

	class Component final {
	public:
		constexpr explicit Component(TransformID id) : ID{ id } {}
		//Creates an entity with an invalid index.
		constexpr Component() : ID{ ID::InvalidID } {}
		constexpr TransformID GetID() const { return ID; }
		constexpr bool IsValid() const { return ID::IsValid(ID); }

		Math::Vector4 GetRotation() const;
		Math::Vector3 GetPosition() const;
		Math::Vector3 GetScale() const;
	private:
		TransformID ID;
	};
}
