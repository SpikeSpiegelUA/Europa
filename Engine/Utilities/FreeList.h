#pragma once
#include "CommonHeaders.h"

namespace Europa::Utilities {
	template<typename T>
	class FreeList {
		static_assert(sizeof(T) >= sizeof(uint32));
	public:
		FreeList() = default;
		explicit FreeList(uint32 count) {
			array.reserve(count);
		}
		~FreeList() {
			assert(!size);
		}
		template<class... params>
		constexpr uint32 Add(params&&... p) {
			uint32 id{ uint32_invalid_id };
			if (nextFreeIndex == uint32_invalid_id) {
				id = (uint32)array.Size();
				array.EmplaceBack(std::forward<params>(p)...);
			}
			else {
				id = nextFreeIndex;
				assert(id < array.Size() && AlreadyRemoved(id));
				nextFreeIndex = *(const uint32* const)std::addressof(array[id]);
				new (std::addressof(array[id])) T(std::forward<params>(p)...);
			}
			++size;
			return id;
		}

		constexpr void Remove(uint32 id) {
			assert(id < array.Size() && !AlreadyRemoved(id));
			T& item{ array[id] };
			item.~T();
			DEBUG_OP(memset(std::addressof(array[id]), 0xcc, sizeof(T)));
			*(uint32* const)std::addressof(array[id]) = nextFreeIndex;
			nextFreeIndex = id;
			--size;
		}

		constexpr uint32 Size() const {
			return size;
		}

		constexpr uint32 Capacity() const {
			return array.Size();
		}

		constexpr bool Empty() const {
			return array.Size() == 0;
		}
		
		[[nodiscard]] constexpr T& operator[](uint32 id){
			assert(id < array.Size() && !AlreadyRemoved(id));
			return array[id];
		}

		[[nodiscard]] constexpr const T& operator[](uint32 id) const{
			assert(id < array.Size() && !AlreadyRemoved(id));
			return array[id];
		}

	private:
		constexpr bool AlreadyRemoved(uint32 id) {
			//Note: when sizeof(T) == sizeof(uint32) we can't test if the item was already removed!
			if constexpr (sizeof(T) > sizeof(uint32)) {
				//Skip the first 4 bytes.
				uint32 i{ sizeof(uint32) };
				const uint8* const p{ (const uint8* const)std::addressof(array[id]) };
				while (p[i] == 0xcc && (i < sizeof(T)))
					i++;
				return i == sizeof(T);
			}
			else
				return true;
		}

		Utilities::Vector<T, false> array; 
		uint32 nextFreeIndex{ uint32_invalid_id };
		uint32 size{ 0 };
	};
}