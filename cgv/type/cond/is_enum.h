#pragma once

#include <type_traits>

namespace cgv {
	namespace type {
		namespace cond {

/// template condition returning, whether the given type is an enum type
template <typename T>
struct is_enum
{ 
	static const bool value =  std::is_enum<T>::value;
};

		}
	}
}
