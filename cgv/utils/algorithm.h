#pragma once

#include <algorithm>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

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

/// @brief Zip two sequences together to form a single sequene of pairs and store the results in an output range starting from d_first.
/// 
/// The sequence beginning at first2 must provide at least distance(first1, last1) - 1 elements.
/// For example, applying zip to sequences (1, 2, 3) and ('a', 'b', 'c') would result in: ((1, 'a'),(2, 'b'),(3, 'c')).
/// 
/// @tparam InputIt1 The first input range iterator type.
/// @tparam InputIt2 The second input range iterator type.
/// @tparam OutputIt The output range iterator type.
/// @param first1 The start of the first input range.
/// @param last1 The end of the first input range.
/// @param first2 The start of the second input range.
/// @param d_first The start of the output range.
/// @return Output iterator to the element that follows the last element transformed.
template<typename InputIt1, typename InputIt2, typename OutputIt>
OutputIt zip(const InputIt1 first1, const InputIt1 last1, const InputIt2 first2, OutputIt d_first) {
	return std::transform(first1, last1, first2, d_first, [](const auto& a, const auto& b) {
		return std::make_pair(a, b);
	});
}

/// @brief Zip two sequences together to form a single sequene of pairs.
/// 
/// The sequence beginning at first2 must provide at least distance(first1, last1) - 1 elements.
/// For example, applying zip to sequences (1, 2, 3) and ('a', 'b', 'c') would return: ((1, 'a'),(2, 'b'),(3, 'c')).
/// 
/// @tparam InputIt1 The first input range iterator type.
/// @tparam InputIt2 The second input range iterator type.
/// @param first1 The start of the first input range.
/// @param last1 The end of the first input range.
/// @param first2 The start of the second input range.
/// @return A std::vector of std::pair s containing copies of zipped elements.
template<typename InputIt1, typename InputIt2>
std::vector<std::pair<typename InputIt1::value_type, typename InputIt2::value_type>> zip(const InputIt1 first1, const InputIt1 last1, const InputIt2 first2) {
	std::vector<std::pair<typename InputIt1::value_type, typename InputIt2::value_type>> res;
	zip(first1, last1, first2, std::back_inserter(res));
	return res;
}

/// @brief Generate a sequence of n uniformly-spaced values in [start,stop] and store the result in an output range starting from output_first.
/// 
/// @tparam ParamT The sequence value type.
/// @tparam OutputIt The output range iterator type.
/// @param output_first The start of the output range.
/// @param operation The operation to transform the sequence. Takes one argument of type ParamT.
/// @param start The starting value of the sequence.
/// @param stop The end value of the sequence.
/// @param n The number of values in the generated sequence.
template<typename ParamT = float, typename OutputIt>
void subdivision_sequence(OutputIt output_first, ParamT start, ParamT stop, size_t n) {
	if(n == 1) {
		*output_first = ParamT(0.5) * (start + stop);
		++output_first;
	} else if(n > 1) {
		const ParamT size = stop - start;
		const ParamT step = size / static_cast<ParamT>(n - 1);
		for(size_t i = 0; i < n; ++i) {
			ParamT t = start + step * static_cast<ParamT>(i);
			*output_first = t;
			++output_first;
		}
	}
}

/// @brief Return a sequence of monotonically increasing values from 0 to n = distance(first, last) - 1 with first == 0 and last == n.
/// 
/// @tparam InputIt The input sequence iterator type.
/// @param first The start of the input range.
/// @param last The end of the input range.
/// @return The index sequence.
template<class InputIt>
std::vector<size_t> generate_index_sequence(const InputIt first, const InputIt last) {
	std::vector<size_t> indices(static_cast<size_t>(std::distance(first, last)));
	std::iota(indices.begin(), indices.end(), 0);
	return indices;
}

/// @brief Return a sequence of indices corresponding to the sorted order of values in [first,last).
/// The value sequence remains unchanged.
/// 
/// @tparam RandomIt The value sequence iterator type.
/// @param first first The start of the value range.
/// @param last The end of the value range.
/// @return The index sequence.
template<class RandomIt>
std::vector<size_t> sort_indices(const RandomIt first, const RandomIt last) {
	std::vector<size_t> indices = generate_index_sequence(first, last);
	std::sort(indices.begin(), indices.end(), [first](size_t i1, size_t i2) {
		return first[i1] < first[i2];
	});
	return indices;
}

/// @brief Return a sequence of indices corresponding to the sorted order of values in [first,last) while preserving the order of equivalent elements.
/// The value sequence remains unchanged.
/// 
/// @tparam RandomIt The value sequence iterator type.
/// @param first first The start of the value range.
/// @param last The end of the value range.
/// @return The index sequence.
template<class RandomIt>
std::vector<size_t> stable_sort_indices(const RandomIt first, const RandomIt last) {
	std::vector<size_t> indices = generate_index_sequence(first, last);
	std::stable_sort(indices.begin(), indices.end(), [first](size_t i1, size_t i2) {
		return first[i1] < first[i2];
	});
	return indices;
}

/// @brief Return a sequence of indices corresponding to the sorted order of values in [first,last).
/// Elements are sorted with respect to comp.
/// The value sequence remains unchanged.
/// 
/// @tparam RandomIt The value sequence iterator type.
/// @tparam Compare The comparison function object type.
/// @param first first The start of the value range.
/// @param last The end of the value range.
/// @param comp The comparison function object.
/// @return The index sequence.
template<class RandomIt, class Compare>
std::vector<size_t> sort_indices(const RandomIt first, const RandomIt last, Compare comp) {
	std::vector<size_t> indices = generate_index_sequence(first, last);
	std::sort(indices.begin(), indices.end(), [first, &comp](size_t i1, size_t i2) {
		return comp(first[i1], first[i2]);
	});
	return indices;
}

/// @brief Return a sequence of indices corresponding to the sorted order of values in [first,last).
/// Elements are sorted with respect to comp while the order of equivalent elements is preserved.
/// The value sequence remains unchanged.
/// 
/// @tparam RandomIt The value sequence iterator type.
/// @tparam Compare The comparison function object type. 
/// @param first first The start of the value range.
/// @param last The end of the value range.
/// @param comp The comparison function object.
/// @return The index sequence.
template<class RandomIt, class Compare>
std::vector<size_t> stable_sort_indices(const RandomIt first, const RandomIt last, Compare comp) {
	std::vector<size_t> indices = generate_index_sequence(first, last);
	std::stable_sort(indices.begin(), indices.end(), [first, &comp](size_t i1, size_t i2) {
		return comp(first[i1], first[i2]);
	});
	return indices;
}

} // namespace utils
} // namespace cgv
