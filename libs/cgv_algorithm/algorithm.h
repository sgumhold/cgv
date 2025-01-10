#pragma once

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <string>
#include <vector>

#include "lib_begin.h"

namespace cgv {
namespace algorithm {

// Dummy method to force generation of lib file.
int CGV_API export_dummy();

/// Unary operator that returns pair.first.
template <class T1 = void, class T2 = void>
struct pair_first {
	constexpr T1 operator()(const std::pair<T1, T2>& pair) const {
		return pair.first;
	}
};

/// Unary operator that returns pair.first.
template <>
struct pair_first<void, void> {
	template <class T1, class T2>
	constexpr auto operator()(const std::pair<T1, T2>& pair) const
		-> decltype(static_cast<const T1&>(pair.first)) {
		return static_cast<const T1&>(pair.first);
	}
};

/// Unary operator that returns pair.second.
template <class T1 = void, class T2 = void>
struct pair_second {
	constexpr T2 operator()(const std::pair<T1, T2>& pair) const {
		return pair.second;
	}
};

/// Unary operator that returns pair.second.
template <>
struct pair_second<void, void> {
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

/// @brief Transform elements in the range [first, last) and concatenate them in a std::string delimited by a separator.
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
std::string transform_join(const InputIt first, const InputIt last, UnaryOp operation, const std::string& separator = ",", bool trailing_separator = false) {
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

// Extended version of enumerate and count ranges.
// Original copyright by https://www.numbercrunch.de/blog/2018/02/range-based-for-loops-with-counters/

// Wraps an iterator into a pair of an iterator and an integer index.
template<typename iterator_type>
class indexed_iterator {
public:
    using iterator = iterator_type;
    using reference = typename std::iterator_traits<iterator>::reference;
    using index_type = typename std::iterator_traits<iterator>::difference_type;
protected:
    iterator iter;
    index_type index = 0;
public:
    indexed_iterator() = delete;
    explicit indexed_iterator(iterator iter, index_type start)
        : iter(iter), index(start) {}
    indexed_iterator& operator++() {
        ++iter;
        ++index;
        return *this;
    }
    bool operator==(const indexed_iterator& other) const {
        return iter == other.iter;
    }
    bool operator!=(const indexed_iterator& other) const {
        return iter != other.iter;
    }
    std::pair<reference, const index_type&> operator*() const {
        return { *iter, index };
    }
};

// Exposes the index of the wrapped iterator.
template<typename iterator_type>
class count_iterator : public indexed_iterator<iterator_type> {
public:
    typedef indexed_iterator<iterator_type> super;
    using iterator = typename super::iterator;
    using index_type = typename super::index_type;
    count_iterator() = delete;
    explicit count_iterator(iterator iter, index_type start)
        : super(iter, start) {}
    index_type operator*() const {
        return super::index;
    }
};

// Pseudo container, wraps a range given by two iterators [first, last)
// into a range of count iterators.
template<typename iterator_type>
class count_range {
public:
    using iterator = count_iterator<iterator_type>;
    using index_type = typename std::iterator_traits<iterator_type>::difference_type;
private:
    const iterator_type first, last;
    const index_type start;
public:
    count_range() = delete;
    explicit count_range(iterator_type first, iterator_type last,
                         index_type start = 0)
        : first(first), last(last), start(start) {}
    iterator begin() const {
        return iterator(first, start);
    }
    iterator end() const {
        return iterator(last, start);
    }
};

// Convert a container into a range of count iterators.
template<typename container_type>
decltype(auto) count(container_type& content,
                     typename std::iterator_traits<typename container_type::iterator>::difference_type start = 0) {
    using ::std::begin;
    using ::std::end;
    return count_range(begin(content), end(content), start);
}

// Convert a range given by two iterators [first, last) into a range
// of count iterators.
template<typename iterator_type>
decltype(auto) count(iterator_type first, iterator_type last,
                     typename std::iterator_traits<iterator_type>::difference_type start = 0) {
    return count_range(first, last, start);
}

// Convert an initializer list into a range of count iterators.
template<typename type>
decltype(auto) count(const std::initializer_list<type>& content,
                     std::ptrdiff_t start = 0) {
    return count_range(content.begin(), content.end(), start);
}

// Convert an rvalue reference to an initializer list into a range of count iterators.
template<typename type>
decltype(auto) count(std::initializer_list<type>&& content,
                     std::ptrdiff_t start = 0) {
    // Overloaded count_range constructor must move the list into the count_range object.
    return count_range(content, start);
}

// Convert a C-array into a range of count iterators.
template<typename type, std::size_t N>
decltype(auto) count(type(&content)[N],
                     std::ptrdiff_t start = 0) {
    return count_range(content, content + N, start);
}

// Exposes the iterator and index of the wrapped iterator.
template<typename iterator_type>
class enumerate_iterator : public indexed_iterator<iterator_type> {
public:
    typedef indexed_iterator<iterator_type> super;
    using iterator = typename super::iterator;
    using index_type = typename super::index_type;
    using reference = typename super::reference;
    enumerate_iterator() = delete;
    explicit enumerate_iterator(iterator iter, index_type start)
        : super(iter, start) {}
    std::pair<reference, const index_type&> operator*() const {
        return { *super::iter, super::index };
    }
};

// Pseudo container, wraps a range given by two iterators [first, last)
// into a range of enumerate iterators.
template<typename iterator_type>
class enumerate_range {
public:
    using iterator = enumerate_iterator<iterator_type>;
    using index_type = typename std::iterator_traits<iterator_type>::difference_type;
private:
    const iterator_type first, last;
    const index_type start;
public:
    enumerate_range() = delete;
    explicit enumerate_range(iterator_type first, iterator_type last,
                             index_type start = 0)
        : first(first), last(last), start(start) {}
    iterator begin() const {
        return iterator(first, start);
    }
    iterator end() const {
        return iterator(last, start);
    }
};

// Convert a container into a range of enumerate iterators.
template<typename container_type>
decltype(auto) enumerate(container_type& content,
                         typename std::iterator_traits<typename container_type::iterator>::difference_type start = 0) {
    using ::std::begin;
    using ::std::end;
    return enumerate_range(begin(content), end(content), start);
}

// Convert a range given by two iterators [first, last) into a range
// of enumerate iterators.
template<typename iterator_type>
decltype(auto) enumerate(iterator_type first, iterator_type last,
                         typename std::iterator_traits<iterator_type>::difference_type start = 0) {
    return enumerate_range(first, last, start);
}

// Convert an initializer list into a range of enumerate iterators.
template<typename type>
decltype(auto) enumerate(const std::initializer_list<type>& content,
                         std::ptrdiff_t start = 0) {
    return enumerate_range(content.begin(), content.end(), start);
}

// Convert an rvalue reference to an initializer list into a range of enumerate iterators.
template<typename type>
decltype(auto) enumerate(std::initializer_list<type>&& content,
                         std::ptrdiff_t start = 0) {
    // Overloaded enumerate_range constructor must move the list into the enumerate_range object.
    return enumerate_range(content, start);
}

// Convert a C-array into a range of enumerate iterators.
template<typename type, std::size_t N>
decltype(auto) enumerate(type(&content)[N],
                         std::ptrdiff_t start = 0) {
    return enumerate_range(content, content + N, start);
}


} // namespace algorithm
} // namespace cgv

#include <cgv/config/lib_end.h>
