#include "Script.h"

namespace Europa::Script {
	namespace {
		Utilities::vector <Internal::ScriptPointer> EntityScripts;
		Utilities::vector<ID::IDType> IDMapping;

		Utilities::vector<ID::GenerationType> Generations;
		Utilities::vector<ScriptID> FreeIDs;

		using ScriptRegistry = std::unordered_map<size_t, Internal::ScriptCreator>;

		ScriptRegistry& Registery() {
			//We put this variable in a function because of a static data initialization order.
			//This way we can be sure, that the order will be correct.
			static ScriptRegistry registery;
			return registery;
		}

		#ifdef USE_WITH_EDITOR
		Utilities::vector<std::string>& ScriptNames() {
			//We put this variable in a function because of a static data initialization order.
			//This way we can be sure, that the order will be correct.
			static Utilities::vector<std::string> names;
			return names;
		}
		#endif

		//Checks if a script's id is valid and if a script at that id is valid.
		bool Exists(ScriptID id) {
			ID::IsValid(id);
			const ID::IDType index{ ID::Index(id) };
			assert(index < Generations.size() && IDMapping[index] < EntityScripts.size());
			assert(Generations[index] == ID::Generation(id));
			return (Generations[index] == ID::Generation(id) && EntityScripts[IDMapping[index]] && EntityScripts[IDMapping[index]]->IsValid());
		}
	} // Anonymous namespace.

	namespace Internal {
		uint8 RegisterScript(size_t tag, ScriptCreator func) {
			bool result{ Registery().insert(ScriptRegistry::value_type{tag,func}).second };
			assert(result);
			return result;
		}

		ScriptCreator GetScriptCreator(size_t tag)
		{
			auto script = Europa::Script::Registery().find(tag);
			assert(script != Europa::Script::Registery().end() && script->first == tag);
			return script->second;
		}

		#ifdef USE_WITH_EDITOR
		uint8 AddScriptName(const char* name) {
			ScriptNames().emplace_back(name);
			return true;
		}
		#endif //USE_WITH_EDITOR.
	} // Script's internal namespace.

	//Creates a new script
	//@param initInfo - initialization information for the script;
	//@param entity - GameEntity that the script belongs to.
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
		const ID::IDType index{ (ID::IDType)EntityScripts.size() - 1 };
		IDMapping[ID::Index(id)] = index;
		return Component{ id };
	}

	//Removes a script from the engine.
	//@param componentToRemove - script(component) to remove.
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

#ifdef USE_WITH_EDITOR
#include <atlsafe.h>

extern "C" __declspec(dllexport)
LPSAFEARRAY GetScriptNames() {
	const uint32 size{ (uint32)Europa::Script::ScriptNames().size() };
	if (!size)
		return nullptr;
	CComSafeArray<BSTR> names(size);
	for (uint32 i{ 0 }; i < size; ++i) {
		names.SetAt(i, A2BSTR_EX(Europa::Script::ScriptNames()[i].c_str()), false);
	}
	return names.Detach();
}
#endif //USE_WITH_EDITOR.
