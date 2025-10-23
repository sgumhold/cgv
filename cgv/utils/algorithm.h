#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include <utility>

//#include "lib_begin.h"

namespace cgv {
namespace utils {

/// Return std::pair::first.
template <class T1 = void, class T2 = void>
struct get_first {
	constexpr T1 operator()(const std::pair<T1, T2>& pair) const {
		return pair.first;
	}
};

/// Return const std::pair::first&.
template <>
struct get_first<void, void> {
	template <class T1, class T2>
	constexpr auto operator()(const std::pair<T1, T2>& pair) const
		-> decltype(static_cast<const T1&>(pair.first)) {
		return static_cast<const T1&>(pair.first);
	}
};

/// Return std::pair::second.
template <class T1 = void, class T2 = void>
struct get_second {
	constexpr T2 operator()(const std::pair<T1, T2>& pair) const {
		return pair.second;
	}
};

/// Return const std::pair::second&.
template <>
struct get_second<void, void> {
	template <class T1, class T2>
	constexpr auto operator()(const std::pair<T1, T2>& pair) const
		-> decltype(static_cast<const T2&>(pair.second)) {
		return static_cast<const T2&>(pair.second);
	}
};

/// @brief Find the minimum value in the range [first, last) or return fallback if the range is empty.
/// 
/// @tparam ForwardIt A forward iterator.
/// @tparam T The range elements type.
/// @param first The start of the range.
/// @param last The end of the range.
/// @param fallback The value returned for an empty range.
/// @return The minimum or fallback value.
template<typename ForwardIt, typename T>
T min_value(ForwardIt first, ForwardIt last, T fallback) {
	auto it = std::min_element(first, last);
	return it != last ? static_cast<T>(*it) : fallback;
}

/// @brief Find the minimum value among the transformed elements in the range [first, last) or return fallback if the range is empty.
/// 
/// @tparam ForwardIt A forward iterator.
/// @tparam UnaryOp A unary operation.
/// @tparam T The range elements type.
/// @param first The start of the range.
/// @param last The end of the range.
/// @param operation The unary operation used to transform input elements to type T before comparison.
/// @param fallback The value returned for an empty range.
/// @return The minimum or fallback value.
template<typename ForwardIt, typename UnaryOp, typename T>
T min_value(ForwardIt first, ForwardIt last, UnaryOp operation, T fallback) {
	auto it = std::min_element(first, last, [&operation](const auto& left, const auto& right) {
		return operation(left) < operation(right);
	});
	return it != last ? static_cast<T>(operation(*it)) : fallback;
}

/// @brief Find the maximum value in the range [first, last) or return fallback if the range is empty.
/// 
/// @tparam ForwardIt A forward iterator.
/// @tparam T The range elements type.
/// @param first The start of the range.
/// @param last The end of the range.
/// @param fallback The value returned for an empty range.
/// @return The maximum or fallback value.
template<typename ForwardIt, typename T>
T max_value(ForwardIt first, ForwardIt last, T fallback) {
	auto it = std::max_element(first, last);
	return it != last ? static_cast<T>(*it) : fallback;
}

/// @brief Find the maximum value among the transformed elements in the range [first, last) or return fallback if the range is empty.
/// 
/// @tparam ForwardIt A forward iterator.
/// @tparam UnaryOp A unary operation.
/// @tparam T The range elements type.
/// @param first The start of the range.
/// @param last The end of the range.
/// @param operation The unary operation used to transform input elements to type T before comparison.
/// @param fallback The value returned for an empty range.
/// @return The maximum or fallback value.
template<typename ForwardIt, typename UnaryOp, typename T>
T max_value(ForwardIt first, ForwardIt last, UnaryOp operation, T fallback) {
	auto it = std::max_element(first, last, [&operation](const auto& left, const auto& right) {
		return operation(left) < operation(right);
	});
	return it != last ? static_cast<T>(operation(*it)) : fallback;
}

/// @brief Transform elements in the range [first, last) and concatenate them to a std::string.
/// 
/// @tparam InputIt An iterator.
/// @tparam UnaryOp A unary operation.
/// @param first The start of the input range.
/// @param last The end of the input range.
/// @param operation The unary operation used to transform the input elements. Result must be or be convertible to std::string.
/// @param separator The separator string used to delimit the concatenated elements.
/// @param trailing_separator If true, separator is added to the end of the returned string.
/// @return The concatenated string.
template <class InputIt, class UnaryOp>
std::string transform_join(const InputIt first, const InputIt last, UnaryOp operation, const std::string& separator, bool trailing_separator = false) {
    std::string res = "";
    if(last > first) {
        for(auto curr = first; curr != last; ++curr) {
            res += operation(*curr);
            if(last - curr > 1 || trailing_separator)
                res += separator;
        }
    }
    return res;
}

/// @brief Concatenate elements in the range [first, last) to a std::string.
/// 
/// Requires to_string method in global namespace to be implemented for InputIt value type.
/// 
/// @tparam InputIt An iterator.
/// @param first The start of the input range.
/// @param last The end of the input range.
/// @param separator The separator string used to delimit the concatenated elements.
/// @param trailing_separator If true, separator is added to the end of the returned string.
/// @return The concatenated string.
template <class InputIt>
std::string join(const InputIt first, const InputIt last, const std::string& separator, bool trailing_separator = false) {
	return transform_join(first, last, [](const auto& val) { return to_string(val); }, separator, trailing_separator);
}

/// @brief Transform the elements in the range [first, last) to adjacent pairs and store the results in an output range starting from d_first.
/// 
/// If the input range contains less than two elements d_first remains unaltered.
/// 
/// @tparam InputIt An iterator.
/// @tparam OutputIt An iterator.
/// @param first The start of the input range.
/// @param last The end of the input range.
/// @param d_first The beginning of the destination range, may be equal to first.
/// @return Output iterator to the element that follows the last element transformed.
template<class InputIt, class OutputIt>
OutputIt pair_adjacent(const InputIt first, const InputIt last, OutputIt d_first) {
	if(std::distance(first, last) > 1) {
		return std::transform(first, std::prev(last), std::next(first), d_first, [](const auto& a, const auto& b) {
			return std::make_pair(a, b);
		});
	}
	return d_first;
}

/// @brief Return a collection of pairwise adjacent elements in the range [first, last).
/// 
/// If the input range contains less than two elements the resulting collection will be empty.
/// 
/// @tparam InputIt An iterator.
/// @param first The start of the input range.
/// @param last The end of the input range.
/// @return A std::vector of std::pair s containing copies of adjacent elements.
template<class InputIt>
std::vector<std::pair<typename InputIt::value_type, typename InputIt::value_type>> pair_adjacent(const InputIt first, const InputIt last) {
	std::vector<std::pair<typename InputIt::value_type, typename InputIt::value_type>> res;
	pair_adjacent(first, last, std::back_inserter(res));
	return res;
}

} // namespace utils
} // namespace cgv

//#include <cgv/config/lib_end.h>
