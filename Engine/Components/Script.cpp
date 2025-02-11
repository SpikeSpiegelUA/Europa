#include "Script.h"

namespace Europa::Script {
	namespace {
		Utilities::vector <Internal::ScriptPointer> EntityScripts;
		Utilities::vector<ID::IDType> IDMapping;

		Utilities::vector<ID::GenerationType> Generations;
		Utilities::vector<ScriptID> FreeIDs;

		using ScriptRegistery = std::unordered_map<size_t, Internal::ScriptCreator>;

		ScriptRegistery& Registery() {
			//We put this variable in a function because of a static data initialization order.
			//This way we can be sure, that the order will be correct.
			static ScriptRegistery registery;
			return registery;
		}

		bool Exists(ScriptID id) {
			ID::IsValid(id);
			const ID::IDType index{ ID::Index(id) };
			assert(index < Generations.size() && IDMapping[index] < EntityScripts.size());
			assert(Generations[index] == ID::Generation(id));
			return (Generations[index] == ID::Generation(id) && EntityScripts[IDMapping[index]] && EntityScripts[IDMapping[index]]->IsValid());
		}
	}

	namespace Internal {
		uint8 RegisterScript(size_t tag, ScriptCreator func) {
			bool result{ Registery().insert(ScriptRegistery::value_type{tag,func}).second };
			assert(result);
			return result;
		}
	}
	Component Europa::Script::Create(const InitInfo& initInfo, GameEntity::Entity entity)
	{
		assert(entity.IsValid());
		assert(initInfo.ScriptCreator);

		ScriptID id{};
		if (FreeIDs.size() > ID::MinDeletedElements) {
			id = FreeIDs.front();
			assert(!Exists(id));
			FreeIDs.pop_back();
			id = ScriptID{ ID::NewGeneration(id) };
			++Generations[ID::Index(id)];
		}
		else {
			id = ScriptID{ (ID::IDType)IDMapping.size() };
			IDMapping.emplace_back();
			Generations.push_back(0);
		}
		assert(ID::IsValid(id));
		EntityScripts.emplace_back(initInfo.ScriptCreator(entity));
		assert(EntityScripts.back()->GetID() == entity.GetID());
		const ID::IDType index{ (ID::IDType)EntityScripts.size() };
		IDMapping[ID::Index(id)] = index;
		return Component{ id };
	}

	void Europa::Script::Remove(Component componentToRemove)
	{
		assert(componentToRemove.IsValid() && Exists(componentToRemove.GetID()));
		const ScriptID id{ componentToRemove.GetID() };
		const ID::IDType index{ IDMapping[ID::Index(id)] };
		const ScriptID lastID{ EntityScripts.back()->GetScript().GetID() };
		Utilities::EraseUnordered(EntityScripts, index);
		IDMapping[ID::Index(lastID)] = index;
		IDMapping[ID::Index(id)] = ID::InvalidID;
	}
}
