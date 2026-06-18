#pragma once

#include <limits>
#include <type_traits>

#define IS_FLOATING_POINT_GUARD typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true

/// This header provides utility functions for comparing floating point numbers.
namespace cgv {
namespace math {

/// @brief Compare two floating point numbers for equality using the machine epsilon as threshold.
/// 
/// @tparam T The floating point data type.
/// @param a The first number.
/// @param b The second number.
/// @return True if the numbers are equal within the accuracy of the machine epsilon, false otherwise.
template<typename T, IS_FLOATING_POINT_GUARD>
bool is_equal(T a, T b) {
	return std::abs(a - b) < std::numeric_limits<T>::epsilon();
}

/// @brief Check if a floating point number is zero using the machine epsilon as threshold.
/// 
/// @tparam T The floating point data type.
/// @param x The number to check.
/// @return True if the given number is less than the machine epsilon, false otherwise.
template<typename T, IS_FLOATING_POINT_GUARD>
bool is_zero(T x) {
	return std::abs(x) < std::numeric_limits<T>::epsilon();
}

} // namespace math
} // namespace cgv

#undef IS_FLOATING_POINT_GUARD
