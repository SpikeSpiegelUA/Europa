#include "TransformComponent.h"
#include "Entity.h"

namespace Europa::TransformComponent {

	namespace {
		Utilities::vector<Math::Vector3> positions;
		Utilities::vector<Math::Vector4> rotations;
		Utilities::vector<Math::Vector3> scales;
	} //Anonymouse namespace to give access only to this translation unit.

	Component Create(const InitInfo& initInfo, GameEntity::Entity entity) {
		assert(entity.IsValid());
		const ID::IDType entityIndex{ ID::Index(entity.GetID()) };
		if (positions.size() > entityIndex) {
			rotations[entityIndex] = Math::Vector4(initInfo.rotation);
			positions[entityIndex] = Math::Vector3(initInfo.position);
			scales[entityIndex] = Math::Vector3(initInfo.scale);
		}
		else {
			assert(positions.size() == entityIndex);
			rotations.emplace_back(initInfo.rotation);
			positions.emplace_back(initInfo.position);
			scales.emplace_back(initInfo.scale);
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
