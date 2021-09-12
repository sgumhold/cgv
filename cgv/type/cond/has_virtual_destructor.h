#pragma once

#include <type_traits>

namespace cgv {
	namespace type {
		/// namespace for conditions that act on types
		namespace cond {
/// template condition returning, whether the passed type has a virtual destructor
template <typename T>
struct has_virtual_destructor
{ 
	static const bool value = std::has_virtual_destructor<T>::value;
};
		}
	}
}
