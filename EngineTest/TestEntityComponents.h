#pragma once
#include "Test.h"
#include "../Engine/Components/Entity.h"
#include "../Engine/Components/TransformComponent.h"

#include <iostream>
#include <ctime>

//Class that tests  game entities related stuff.
class EngineTest : public Test {
public:
	bool Initialize() override {
		srand((uint32)time(nullptr));
		return true;
	}
	void Run() override {
		do {
			for (uint32 index = 0; index < 10000; ++index) {
				CreateRandom();
				RemoveRandom();
				NumberOfEntities = (uint32)Entities.size();
			}
			PrintResults();
		} while (getchar() != 'q');
	}
	void Shutdown() override {

	}
private:
	//Create random number of entities for the test. If the Entities vector is empty, this function will create 1000 entities.
	void CreateRandom() {
		uint32 count = rand() % 20;
		if (Entities.empty())
			count = 1000;
		Europa::TransformComponent::InitInfo transformComponentInitInfo{};
		Europa::GameEntity::EntityInfo entityInfo{
			&transformComponentInitInfo
		};
		while (count > 0) {
			++Added;
			Europa::GameEntity::Entity entity{ Europa::GameEntity::CreateGameEntity(entityInfo) };
			assert(entity.IsValid() && Europa::ID::IsValid(entity.GetID()));
			Entities.push_back(entity);
			assert(Europa::GameEntity::IsAlive(entity));
			--count;
		}
	}
	//Remove a random number of entities. If the entities count is less than 1000, return.
	void RemoveRandom() {
		uint32 count = rand() % 20;
		if (Entities.size() < 1000)
			return;
		while (count > 0) {
			const uint32 index{ (uint32)rand() % (uint32)Entities.size() };
			const Europa::GameEntity::Entity entity{ Entities[index] };
			assert(entity.IsValid() && Europa::ID::IsValid(entity.GetID()));
			if (entity.IsValid()) {
				Europa::GameEntity::RemoveGameEntity(entity);
				Entities.erase(Entities.begin() + index);
				assert(!Europa::GameEntity::IsAlive(entity));
				++Removed;
			}
			--count;
		}
	}

	void PrintResults() {
		std::cout << "Entities created: " << Added << '\n';
		std::cout << "Entities deleted: " << Removed << '\n';
		std::cout << "Total amount of entities: " << NumberOfEntities << '\n';
	}

	Europa::Utilities::vector<Europa::GameEntity::Entity> Entities;

	uint32 Added{ 0 };
	uint32 Removed{ 0 };
	uint32 NumberOfEntities{ 0 };
};
