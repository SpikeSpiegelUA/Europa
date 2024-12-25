#include "Entity.h"
#include <vector>

namespace Europa::GameEntity {
	//Anonymous namespace, to make these functions and variables inaccessible outside this translation unit.
	namespace {
		Utilities::vector<ID::GenerationType> Generations;
		Utilities::deque<EntityID> FreeIDs;
	}
	Entity CreateGameEntity(const EntityInfo& entityInfo)
	{
		assert(entityInfo.Transform);//All game entities must have a transform component.
		if (!entityInfo.Transform)
			return Entity{};
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
		}

		const Entity NewEntity{ newID };
		const ID::IDType Index{ ID::Index(newID) };

		return NewEntity;
	}
	void RemoveGameEntity(Entity entity)
	{
		const EntityID  entityID{ entity.GetID() };
		const ID::IDType index{ ID::Index(entityID) };
		assert(IsAlive(entity));
		if (IsAlive(entity)) {
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
		return (Generations[index] == ID::Generation(entityID));
	}
}
