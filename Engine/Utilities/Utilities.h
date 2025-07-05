#pragma once

#define USE_STL_VECTOR 1
#define USE_STL_DEQUE 1

#if USE_STL_VECTOR
#include <vector>
#include <algorithm>
namespace Europa::Utilities {
	template<typename T>
	using Vector = typename std::vector<T>;
}
#endif

#if USE_STL_DEQUE
#include <deque>
namespace Europa::Utilities {
	template<typename T>
	using deque = typename std::deque<T>;

	template<typename T>
	void EraseUnordered(std::vector<T>& vector, size_t index) {
		if (vector.size() > 1) {
			std::iter_swap(vector.begin() + index, vector.end() - 1);
			vector.pop_back();
		}
		else {
			vector.clear();
		}
	}
}
#endif

namespace Europa::Utilities {
	//TODO: Implement our own containers.
}