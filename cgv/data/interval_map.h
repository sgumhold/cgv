#pragma once

#include <map>

namespace cgv {
namespace data {

/// @brief Extension of a standard map container that allows easy retreival of lower and upper bounds given a key.
/// @tparam Key the key type
/// @tparam T the value type
template<class Key, class T>
class interval_map {
public:
	using base_type = std::map<Key, T>;
	using key_type = Key;
	using mapped_type = T;
	using value_type = typename base_type::value_type;
	using size_type = typename base_type::size_type;
	using iterator = typename base_type::iterator;
	using const_iterator = typename base_type::const_iterator;
	using reverse_iterator = typename base_type::reverse_iterator;
	using const_reverse_iterator = typename base_type::const_reverse_iterator;

private:
	base_type base;

	const_iterator lower_bound_impl(key_type key) const {

		const_iterator it = base.lower_bound(key);

		if(it == base.end())
			return base.empty() ? base.end() : std::prev(it);
		else if(it->first == key)
			return it;
		else if(it == base.begin())
			return base.end();
		else
			return std::prev(it);
	}

public:
	void clear() { base.clear(); }

	iterator begin() { return base.begin(); }

	const_iterator begin() const { return base.begin(); }

	iterator end() { return base.end(); }

	const_iterator end() const { return base.end(); }

	reverse_iterator rbegin() { return base.rbegin(); }

	const_reverse_iterator rbegin() const { return base.rbegin(); }

	reverse_iterator rend() { return base.rend(); }

	const_reverse_iterator rend() const { return base.rend(); }

	const_iterator cbegin() const { return base.cbegin(); }

	const_iterator cend() const { return base.cend(); }

	const_reverse_iterator crbegin() const { return base.crbegin(); }

	const_reverse_iterator crend() const { return base.crend(); }

	size_type size() const {

		return base.size();
	}

	bool empty() const {

		return base.empty();
	}

	std::pair<iterator, bool> insert(key_type key, const mapped_type& value) {

		return base.insert({ key, value });
	}

	size_type erase(key_type frame) {

		return base.erase(frame);
	}

	iterator erase(iterator it) {

		return base.erase(it);
	}

	iterator find(key_type key) {

		return base.find(key);
	}

	/// Returns an iterator pointing to the first element in the container whose key is equivalent or smaller than the given key. If no such element exists, returns end().
	iterator lower_bound(key_type key) {

		const_iterator it = lower_bound_impl(key);
		return it == base.end() ? base.end() : base.find(it->first);
	}

	/// Returns a const iterator pointing to the first element in the container whose key is equivalent or smaller than the given key. If no such element exists, returns end().
	const_iterator lower_bound(key_type key) const {

		return lower_bound_impl(key);
	}

	/// Returns an iterator pointing to the first element in the container whose key is greater than the given key. If no such element exists, returns end().
	iterator upper_bound(key_type key) {

		return base.upper_bound(key);
	}

	/// Returns a const iterator pointing to the first element in the container whose key is greater than the given key. If no such element exists, returns end().
	const_iterator upper_bound(key_type key) const {

		return base.upper_bound(key);
	}

	/// Returns a pair of iterators pointing to the lower and upper bounds of the given key.
	/// The lower bound is defined as the greatest stored key less than or equal to the given key.
	/// The upper bound is defined as the smallest stored key greater than the given key.
	std::pair<iterator, iterator> bounds(key_type key) {

		return { lower_bound(key), upper_bound(key) };
	}

	/// Returns a pair of const iterators pointing to the lower and upper bounds of the given key.
	/// The lower bound is defined as the greatest stored key less than or equal to the given key.
	/// The upper bound is defined as the smallest stored key greater than the given key.
	std::pair<const_iterator, const_iterator> bounds(key_type key) const {

		return { lower_bound(key), upper_bound(key) };
	}
};

} // namespace data
} // namespace cgv
