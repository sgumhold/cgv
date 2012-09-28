#pragma once

#include <cgv/type/standard_types.h>
#include <climits>
#include <float.h>

// FIXME: We should really take the standards compilant way which is
// to use INT8_MAX,... instead of _I8_MAX and add compatibility definitions
// for MSVC instead the other way around...
// FIXME: Can't we just use std::numeric_limits?
#ifndef _MSC_VER
// FIXME: better use stdint.h
# if __WORDSIZE == 64
#  define __INT64_C(c)	c ## L
#  define __UINT64_C(c)	c ## UL
# else
#  define __INT64_C(c)	c ## LL
#  define __UINT64_C(c)	c ## ULL
# endif

#define _I8_MAX (127)
#define _I16_MAX (32767)
#define _I32_MAX (2147483647)
#define _I64_MAX (__INT64_C(9223372036854775807))
#define _UI8_MAX (255)
#define _UI16_MAX (65535)
#define _UI32_MAX (4294967295U)
#define _UI64_MAX (__UINT64_C(18446744073709551615))
#endif


namespace cgv {
	namespace type {
		namespace traits {
			/** the max traits defines for each type in the static const member \c value, 
			    what the maximum value is. */
			template <typename T> struct max {};
			template <> struct max<int8_type>  { static const int8_type  value = _I8_MAX; };
			template <> struct max<int16_type> { static const int16_type value = _I16_MAX; };
			template <> struct max<int32_type>   { static const int32_type   value = _I32_MAX; };
			template <> struct max<int64_type>  { static const int64_type value = _I64_MAX; };
			template <> struct max<uint8_type>  { static const uint8_type  value = _UI8_MAX; };
			template <> struct max<uint16_type> { static const uint16_type value = _UI16_MAX; };
			template <> struct max<uint32_type>   { static const uint32_type   value = _UI32_MAX; };
			template <> struct max<uint64_type>  { static const uint64_type value = _UI64_MAX; };

			/** the max_fct traits provides a static function returning the maximal value of
			    a type. This trait also supports floating point types. */
			template <typename T> struct max_fct { static T get_value() { return max<T>::value; } };
			template <> struct max_fct<float>    { static float  get_value() { return FLT_MAX; } };
			template <> struct max_fct<double>   { static double get_value() { return  DBL_MAX; } };
		}
	}
}
