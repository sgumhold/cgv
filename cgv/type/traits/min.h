#pragma once

#include <climits>
#include <float.h>

namespace cgv {
	namespace type {
		namespace traits {
			/** the min traits defines for each type in the static const member \c value, 
			    what the minimum value is. */
			template <typename T> struct min {};
			template <> struct min<char>  { static const char  value = CHAR_MIN; };
			template <> struct min<short> { static const short value = SHRT_MIN; };
			template <> struct min<int>   { static const int   value = INT_MIN; };
			template <> struct min<long>  { static const long  value = LONG_MIN; };
			template <> struct min<unsigned char>  { static const unsigned char  value = 0; };
			template <> struct min<unsigned short> { static const unsigned short value = 0; };
			template <> struct min<unsigned int>   { static const unsigned int   value = 0; };
			template <> struct min<unsigned long>  { static const unsigned long  value = 0; };

			/** the min_fct traits provides a static function returning the minimal value of
			    a type. This trait also supports floating point types. */
			template <typename T> struct min_fct { static T get_value() { return min<T>::value; } };
			template <> struct min_fct<float>    { static float  get_value() { return -FLT_MAX; } };
			template <> struct min_fct<double>   { static double get_value() { return  -DBL_MAX; } };
		}
	}
}