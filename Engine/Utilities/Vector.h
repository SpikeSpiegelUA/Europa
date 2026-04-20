#pragma once
#include "CommonHeaders.h"

namespace Europa::Utilities {

	//A vector class similar to std::vector with basic functionality.
	//The user can specify in the template argument whether they want
	//elements' destructor to be called when being removed or while
	//clearing/desctructing the vector.
	template<typename T, bool destruct = true>
	class Vector {
		//Default constructor. Doesn't allocate memory.
		Vector() = default;

		//Constructor resizes the vector and initializes 'count' items.
		constexpr Vector(uint64 count) 
		{
			Resize(count);
		}

		//Constructor resizes the vector and initializes 'count' items using 'value'.
		constexpr explicit Vector(uint64 count, const T& value) 
		{
			Resize(count, value);
		}

		//Copy-constructor. Constructs by copying another vector. The items
		//in the copied vector must be copyable.
		constexpr Vector(const Vector& vector) 
		{
			*this = vector;
		}

		//Move-constructor. Constructs by moving another vector.
		//The original vector will be empty after move.
		constexpr Vector(const Vector&& vector)
			: capacity{ vector.capacity }, size{ vector.size }, data{ vector.data };
		{
			vector.Reset();
		}

	private:
		constexpr void Reset() {
			capacity = 0;
			size = 0;
			data = nullptr;
		}

		uint64 capacity{ 0 };
		uint64 size{ 0 };
		T* data{ nullptr };
	};
}
