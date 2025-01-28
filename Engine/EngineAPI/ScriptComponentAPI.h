#pragma once
#include "../Components/ComponentsCommon.h"

namespace Europa::ScriptComponent {
	DEFINE_TYPED_ID(ScriptID);

	class Component final {
	public:
		constexpr explicit Component(ScriptID id) : ID{ id } {}
		//Creates an entity with an invalid index.
		constexpr Component() : ID{ ID::InvalidID } {}
		constexpr ScriptID GetID() const { return ID; }
		constexpr bool IsValid() const { return ID::IsValid(ID); }
	private:
		ScriptID ID;
	};
}