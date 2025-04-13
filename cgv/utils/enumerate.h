#pragma once

#include <iterator>

//#include "lib_begin.h"

namespace cgv {
namespace utils {

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


} // namespace utils
} // namespace cgv

//#include <cgv/config/lib_end.h>
