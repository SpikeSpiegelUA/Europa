#include "Entity.h"
#include <vector>
#include "TransformComponent.h"

namespace Europa::GameEntity {
	//Anonymous namespace, to make these functions and variables inaccessible outside this translation unit.
	namespace {
		Utilities::vector<ID::GenerationType> Generations;
		Utilities::deque<EntityID> FreeIDs;
		Utilities::vector<TransformComponent::Component> TransformComponents;
	}
	Entity CreateGameEntity(const EntityInfo& entityInfo)
	{
		assert(entityInfo.Transform);//All game entities must have a transform component.
		if (!entityInfo.Transform)
			return {};
		EntityID newID{};
		if (FreeIDs.size() > ID::MinDeletedElements) {
			newID = FreeIDs.front();
			assert(!IsAlive(Entity{ newID }));
			FreeIDs.pop_front();
			newID = EntityID{ ID::NewGeneration(newID) };
			++Generations[ID::Index(newID)];
		}
		else {
			newID = EntityID{ (ID::IDType)Generations.size()};
			Generations.push_back(0);

			//Resize components.
			//NOTE: we don't call resize(), cause it will reallocate memory exactly for the number of elements we have in the vector,
			//and we won't to reserve more memory for future elements.

			TransformComponents.emplace_back();
		}

		const Entity newEntity{ newID };
		const ID::IDType Index{ ID::Index(newID) };

		//Create transform component.
		assert(!TransformComponents[Index].IsValid());
		TransformComponents[Index] = TransformComponent::CreateTransform(*entityInfo.Transform, newEntity);
		if (!TransformComponents[Index].IsValid())
			return {};

		return newEntity;
	}
	void RemoveGameEntity(Entity entity)
	{
		const EntityID  entityID{ entity.GetID() };
		const ID::IDType index{ ID::Index(entityID) };
		assert(IsAlive(entity));
		if (IsAlive(entity)) {
			TransformComponent::RemoveTransformComponent(TransformComponents[index]);
			TransformComponents[index] = TransformComponent::Component{};
			FreeIDs.push_back(entityID);
		}
	}
	bool IsAlive(Entity entity)
	{
		assert(entity.IsValid());
		const EntityID entityID{ entity.GetID() };
		const ID::IDType index{ ID::Index(entityID) };
		assert(index < Generations.size());
		assert(Generations[index] == ID::Generation(entityID));
		return (Generations[index] == ID::Generation(entityID) && TransformComponents[index].IsValid());
	}

	TransformComponent::Component Entity::GetTransform() const {
		assert(IsAlive(*this));
		const ID::IDType index{ ID::Index(ID) };
		return TransformComponents[index];
	}
}
