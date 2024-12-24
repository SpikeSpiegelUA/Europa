#pragma once
#include "CommonHeaders.h"

namespace Europa::ID {
	using IDType = uint32;

	//ID has a generation part and an index part.


	//Generation is used to determite, whether there were an entity in this index, that got deleted. Increment it by 1 if you
	//are writing a new game entity in this index.
	constexpr uint32 GenerationBits{ 8 };
	//Index part is just an index of a game entity in the game entities array.
	constexpr uint32 IndexBits{ sizeof(IDType) * 8 - GenerationBits };

	//Mask for the index part of the ID. Internally it subtracts 1 to flip all the bits.
	constexpr IDType IndexMask{ (IDType{1} << IndexBits) - 1};
	//Mask for the generation part of the ID. Internally it subtracts 1 to flip all the bits.
	constexpr IDType GenerationMask{ (IDType{1} << GenerationBits) - 1 };
	constexpr IDType IDMask{ IDType{-1} };

	//This is a std::contidional that checks the value of the GenerationBits variable and sets the appropriate type for that size.
	using GenerationType = std::conditional_t<GenerationBits <= 16, std::conditional_t<GenerationBits <= 8, uint8, uint16>, uint32>;
	static_assert(sizeof(GenerationType) * 8 >= GenerationBits);
	static_assert(sizeof(IDType) - sizeof(GenerationType) > 0);

	//Check if an ID is valid.
	inline bool IsValid(IDType id) {
		return id != IDMask;
	}

	//Get the index mask of an ID.
	inline IDType Index(IDType id) {
		return id & IndexMask;
	}

	//Get the generation mask of an ID.
	inline IDType Generation(IDType id) {
		return (id >> IndexBits) & GenerationMask;
	}

	//Increment the generation of an id by 1.
	inline IDType NewGeneration(IDType id) {
		const IDType generation{ ID::Generation(id) + 1 };
		assert(generation < 255);
		return Index(id) | (generation << IndexBits);
	}

//These macros helps us to create IDs for new components and entities. For example, TransformComponentID, GameEntityID.
#if _DEBUG
	//Base struct for all the IDs that we create with DEFINE_TYPED_ID macro.
	namespace Internal {
		struct IDBase {
			constexpr explicit IDBase(IDType id) : ID(id) {};
			constexpr operator IDType() const { return ID; };
		private:
			IDType ID;
		};
	}
//Use this macro to define specific IDs. Simply for convenience.
#define DEFINE_TYPED_ID(name)										\
	struct name final : ID::Internal::IDBase{	                    \
		constexpr explicit name(ID::IDType id) : IDBase{id} {}      \
        constexpr name() : IDBase{ID::IDMask} {}                    \
	};																			
#else
//If we use the release build, just set the specific ID to IDType.
#define DEFINE_TYPED_ID(name) using name = ID::IDType;
#endif
}