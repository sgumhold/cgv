#pragma once

#include <type_traits>

#include "fvec.h"

#define IS_INTEGRAL_GUARD typename std::enable_if<std::is_integral<T>::value, bool>::type = true

/// This header provides implementation for integer arithmetic.
namespace cgv {
namespace math {

/// @brief Return the lowest power of two greater than or equal to x.
template<typename T, IS_INTEGRAL_GUARD>
T next_power_of_two(T x) {
	T pot = T(1);
	while(x > pot)
		pot <<= T(1);
	return pot;
}

/// @brief Return the lowest multiple of k greater than or equal to n.
template<typename T, IS_INTEGRAL_GUARD>
T next_multiple_k_greater_than_n(T k, T n) {
	T remainder = k - (n % k);
	if(n % k == T(0))
		remainder = T(0);
	return n + remainder;
}

/// @brief Return the integer equivalent of ceil(a/b).
template<typename T, IS_INTEGRAL_GUARD>
T div_round_up(T a, T b) {
	return (a + b - T(1)) / b;
}

template<typename T, cgv::type::uint32_type N, IS_INTEGRAL_GUARD>
fvec<T, N> div_round_up(fvec<T, N> a, fvec<T, N> b) {
	return (a + b - T(1)) / b;
}

} // namespace math
} // namespace cgv

#undef IS_INTEGRAL_GUARD
