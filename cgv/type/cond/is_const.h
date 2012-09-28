#pragma once 

namespace cgv {
	namespace type {
		namespace cond {
			/** checks if a type has the const modifier */
			template <typename T>
			struct is_const { static const bool value = false; };
			template <typename T>
			struct is_const<const T> { static const bool value = true; };
			template <typename T>
			struct is_const<T&> : public is_const<T> {};
		}
	}
}
