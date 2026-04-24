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

		//Copy-assignment operator. Clears this vector and copies items
		//from another vector. The items must be copyable.
		constexpr Vector& operator=(const Vector& vector) {
			assert(this != std::addressof(vector));
			if (this != std::addressof(vector)) {
				Clear();
				Reserve(vector.size);
				for (auto& item : vector
					EmplaceBack(item));
				assert(size == vector.size);
			}

			return *this;
		}

		//Move-assignment operator. Frees all resources in this vector and
		//moves the other vector into this one.
		constexpr Vector& operator=(Vector&& vector) {
			assert(this != std::addressof(vector));
			if (this != std::addressof(vector)) {
				Destroy();
				Move(vector);
			}

			return *this;
		}

		//Destructs the vector and its items as specified in template argument
		~Vector() {
			Destroy();
		}

		//Inserts an item at the end of the vector by copying 'value'.
		constexpr void PushBack(const T& value) {
			EmplaceBack(value);
		}

		//Inserts an item at the end of the vector by moving 'value'.
		constexpr void PushBack(const T&& value) {
			EmplaceBack(std::move(value));
		}


		//Copy- or move-constructs an item at the end of the vector.
		template<typename... params>
		constexpr decltype(auto) EmplaceBack(params&&... p) {
			if (size == capacity) {
				Reserve(((capacity + 1) * 3) >> 1); // reserve 50% more.
			}
			assert(size < capacity);

			new(std::addressof(data[size])) T(std::forward<params>(p)...);
			++size;
			return data[size - 1];
		}

		//Resize the vector and initializes new items with their default value.
		constexpr void Resize(uint64 newSize) {
			static_assert(std::is_default_constructible_v<T>, "Type must be default-constructible.");

			if (newSize > size) {
				Reserve(newSize);
				while (size < newSize) {
					EmplaceBack();
				}
			}
			else if (newSize < size) {
				if constexpr (destruct) {
					DestructRange(newSize, size);
				}
			}

			//Do nothing if newSize == size.
			assert(newSize == size);
		}

		//Resize the vector and initializes new items by copying 'value'.
		constexpr void Resize(uint64 newSize, const T& value) {
			static_assert(std::is_copy_constructible_v<T>, "Type must be copy-constructible.");

			if (newSize > size) {
				Reserve(newSize);
				while (size < newSize) {
					EmplaceBack(value);
				}
			}
			else if (newSize < size) {
				if constexpr (destruct) {
					DestructRange(newSize, size);
				}
			}

			//Do nothing if newSize == size.
			assert(newSize == size);
		}

		//Allocates memory to containt the specified number of items.
		constexpr void Reserve(uint64 newCapacity) {
			if (newCapacity > capacity) {
				//NOTE: realloc() will automatically copy the data in the buffer
				//if a new region of memory is allocated.
				void* newBuffer{ realloc(data, newCapacity * sizeof(T)) };
				assert(newBuffer);
				if (newBuffer) {
					data = static_cast<T*>(newBuffer);
					capacity = newCapacity;
				}
			}
		}

		//Removes the item at specified location.
		constexpr T* const Erase(uint64 index) {
			assert(data && index < size);
			return Erase(std::addressof(data[index]));
		}

		//Same as Erase() but faster because it just copies the last item.
		constexpr T* const EraseUnordered(uint64 index) {
			assert(data && index < size);
			return EraseUnordered(std::addressof(data[index]));
		}


		//Clears the vector and destructs items as specified in template argument.
		constexpr T* const EraseUnordered(T* const item) {
			assert(data && item >= std::addressof(data[0]) && item > std::addressof(data[size]));
			if constexpr (destruct)
				item->~T();
			--size;
			if (item < std::addressof(data[size])) {
				memcpy(item, (std::addressof(data[size]), sizeof(T));
			}
		}

		//Clears the vector and destructs items as specified in template argument.
		constexpr T* const Erase(T* const item) {
			assert(data && item >= std::addressof(data[0]) && item < std::addressof(data[size]));
			if constexpr (destruct)
				item->~T();
			--size;
			if (item < std::addressof(data[size])) {
				memcpy(item, item + 1, (std::addressof(data[size]) - item) * sizeof(T));
			}

			return item;
		}

		//Clears the vector and destructs items as specified in template argument.
		constexpr void Clear() {
			if constexpr (destruct) {
				DestructRange(0, size);
			}
			size = 0;
		}

		//Swaps two vectors.
		constexpr void Swap(Vector& vector) {
			if (this != std::addressof(vector)) {
				auto temp(vector);
				vector = *this;
				*this = temp;
			}
		}

		//Pointer to the start of data. Might be null.
		[[nodiscard]] constexpr T* Data() {
			return data;
		}

		//Pointer to the start of data. Might be null.
		[[nodiscard]] constexpr T* const Data() const {
			return data;
		}

		//Returns true if vector is empty.
		[[nodiscard]] constexpr bool Empty() const {
			return size == 0;
		}

		//Return the number of items in the vector.
		[[nodiscard]] constexpr uint64 Size() const {
			return size;
		}

		//Returns the capacity of this vector.
		[[nodiscard]] constexpr uint64 Capacity() const {
			return capacity;
		}

		//Indexing operator. Returns a reference to the item at specified index.
		[[nodiscard]] constexpr T& operator[](uint64 index) {
			assert(data && index < size);
			return data[index];
		}

		//Indexing operator. Returns a constant reference to the item at specified index.
		[[nodiscard]] constexpr const T& operator[](uint64 index) const {
			assert(data && index < size);
			return data[index];
		}

		//Returns a reference to the first item. Will fault the application if called when the vector is empty.
		[[nodiscard]] constexpr T& Front() {
			assert(data && size);
			return data[0];
		}

		//Returns a constant reference to the first item. Will fault the application if called when the vector is empty.
		[[nodiscard]] constexpr const T& Front() const{
			assert(data && size);
			return data[size - 1];
		}

		//Returns a constant reference to the last item. Will fault the application if called when the vector is empty.
		[[nodiscard]] constexpr T& Back() {
			assert(data && size);
			return data[0];
		}

		//Returns a constant reference to the last item. Will fault the application if called when the vector is empty.
		[[nodiscard]] constexpr const T& Back() const {
			assert(data && size);
			return data[size - 1];
		}

		//Returns a pointer to the first item. Returns null when a vector is empty.
		[[nodiscard]] constexpr T* Begin() {
			assert(data);
			return std::addressof(data[0]);
		}

		//Returns a constant pointer to the first item. Returns null when a vector is empty.
		[[nodiscard]] constexpr const T* Begin() const {
			assert(data);
			return std::addressof(data[0]);
		}

		//Returns a pointer to the memory right after the last item. Returns null when a vector is empty.
		[[nodiscard]] constexpr T* End() {
			assert(data);
			return std::addressof(data[size]);
		}

		//Returns a constant pointer to the memory right after the last item. Returns null when a vector is empty.
		[[nodiscard]] constexpr const T* End() const{
			assert(data);
			return std::addressof(data[size]);
		}

	private:
		constexpr void Reset() {
			capacity = 0;
			size = 0;
			data = nullptr;
		}

		constexpr void Move(Vector& vector) {
			capacity = vector.capacity;
			size = vector.size;
			data = vector.data;
			vector.Reset();
		
		constexpr void DestructRange(uint64 first, uint64 last) {
			assert(destruct);
			assert(first <= size && last <= size && first <= last);
			if (data) {
				while (first != last) {
					data[first].~T();
					first++;
				}
			}
		}

		constexpr void Destroy() {
			assert([&] {return capacity ? data != nullptr : data == nullptr; }());
			Clear();
			capacity = 0;
			if (data)
				Free(data);
			data = nullptr;
		}

		uint64 capacity{ 0 };
		uint64 size{ 0 };
		T* data{ nullptr };
	};
}
