#pragma once

#include <cgv/type/traits/max.h>

namespace cgv {
	namespace type {
		namespace traits {
			/** the one traits defines for each type in the static const member \c value, 
			    what the value of one is. */
			template <typename T> struct one { static const T value = max<T>::value; };

			/** the one_fct traits provides a static function returning the value of
			    a type that is interpreted as one. This trait also supports floating point types. */
			template <typename T> struct one_fct { static T get_value() { return one<T>::value; } };
			template <> struct one_fct<float>    { static float  get_value() { return 1.0f; } };
			template <> struct one_fct<double>   { static double get_value() { return  1.0; } };
		}
	}
}
