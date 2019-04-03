#pragma once

namespace cgv {
	namespace type {

/// use this type to mark the result of an invalid meta function
struct invalid_type {};

template <typename T>
struct is_valid
{
	static const int value = 1;
};
template <>
struct is_valid<invalid_type>
{
	static const int value = 0;
};

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
struct count_valid_types
{
	static const int value = is_valid<T1>::value + is_valid<T2>::value + is_valid<T3>::value + is_valid<T4>::value + is_valid<T5>::value + is_valid<T6>::value + is_valid<T7>::value + is_valid<T8>::value;
};

	}
}
