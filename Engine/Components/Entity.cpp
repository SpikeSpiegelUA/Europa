#include "Entity.h"
#include <vector>
#include "TransformComponent.h"

namespace Europa::GameEntity {
	//Anonymous namespace, to make these functions and variables inaccessible outside this translation unit.
	namespace {
		Utilities::vector<ID::GenerationType> Generations;
		Utilities::deque<EntityID> FreeIDs;
		Utilities::vector<TransformComponent::Component> TransformComponents;
		Utilities::vector<Script::Component> Scripts;
	}
	Entity Create(const EntityInfo& entityInfo)
	{
		assert(entityInfo.Transform);//All game entities must have a transform component.
		if (!entityInfo.Transform)
			return {};
		EntityID newID{};
		if (FreeIDs.size() > ID::MinDeletedElements) {
			newID = FreeIDs.front();
			assert(!IsAlive(newID));
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
		const ID::IDType index{ ID::Index(newID) };

		//Create transform component.
		assert(!TransformComponents[index].IsValid());
		TransformComponents[index] = TransformComponent::Create(*entityInfo.Transform, newEntity);
		if (!TransformComponents[index].IsValid())
			return {};

		//Create script component.
		if (entityInfo.Script && entityInfo.Script->ScriptCreator) {
			assert(!Scripts[index].IsValid());
			Scripts[index] = Script::Create(*entityInfo.Script, newEntity);
			assert(Scripts[index].IsValid());
		}

		return newEntity;
	}
	void Remove(EntityID entityID)
	{
		const ID::IDType index{ ID::Index(entityID) };
		assert(ID::IsValid(entityID));
		TransformComponent::Remove(TransformComponents[index]);
		TransformComponents[index] = TransformComponent::Component{};
		FreeIDs.push_back(entityID);
	}
	bool IsAlive(EntityID entityID)
	{
		assert(ID::IsValid(entityID));
		const ID::IDType index{ ID::Index(entityID) };
		assert(index < Generations.size());
		assert(Generations[index] == ID::Generation(entityID));
		return (Generations[index] == ID::Generation(entityID) && TransformComponents[index].IsValid());
	}

	TransformComponent::Component Entity::GetTransform() const {
		assert(IsAlive(ID));
		const ID::IDType index{ ID::Index(ID) };
		return TransformComponents[index];
	}
	Script::Component Entity::GetScript() const
	{
		assert(IsAlive(ID));
		const ID::IDType index{ ID::Index(ID) };
		return Scripts[index];
	}
}
