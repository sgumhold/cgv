#pragma once

#include <type_traits>

namespace cgv {
	namespace type {
		namespace cond {

/// template condition returning, whether the first argument is a base class of the second argument
template <typename T>
struct is_abstract
{ 
	static const bool value =  std::is_abstract<T>::value;
};

		}
	}
}
