#include "TransformComponent.h"
#include "Entity.h"

namespace Europa::TransformComponent {

	namespace {
		Utilities::Vector<Math::Vector3> positions;
		Utilities::Vector<Math::Vector4> rotations;
		Utilities::Vector<Math::Vector3> scales;
	} //Anonymouse namespace to give access only to this translation unit.

	Component Create(const InitInfo& initInfo, GameEntity::Entity entity) {
		assert(entity.IsValid());
		const ID::IDType entityIndex{ ID::Index(entity.GetID()) };
		if (positions.Size() > entityIndex) {
			rotations[entityIndex] = Math::Vector4(initInfo.rotation);
			positions[entityIndex] = Math::Vector3(initInfo.position);
			scales[entityIndex] = Math::Vector3(initInfo.scale);
		}
		else {
			assert(positions.Size() == entityIndex);
			rotations.EmplaceBack(initInfo.rotation);
			positions.EmplaceBack(initInfo.position);
			scales.EmplaceBack(initInfo.scale);
		}
		return Component{ TransformID{entity.GetID()} };
	}

	void Remove(Component componentToRemove) {
		assert(componentToRemove.IsValid());
	}

	Math::Vector4 Component::GetRotation() const {
		assert(IsValid());
		return rotations[ID::Index(ID)];
	}
	Math::Vector3 Component::GetPosition() const {
		assert(IsValid());
		return positions[ID::Index(ID)];
	}
	Math::Vector3 Component::GetScale() const {
		assert(IsValid());
		return scales[ID::Index(ID)];
	}
}
