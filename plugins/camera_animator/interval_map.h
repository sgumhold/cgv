#pragma once

#include <map>

//! Extension of a standard map container that allows easy retreival of lower and upper bounds given a key.
/*! */
template<class key_type, class value_type>
class interval_map {
private:
	typedef typename std::map<key_type, value_type> map_type;
	typedef typename map_type::size_type size_type;
	typedef typename map_type::iterator iterator_type;
	typedef typename map_type::const_iterator const_iterator_type;
	typedef typename map_type::reverse_iterator reverse_iterator_type;
	typedef typename map_type::const_reverse_iterator const_reverse_iterator_type;

	map_type data;

public:
	void clear() {
		data.clear();
	}

	bool empty() const {

		return data.empty();
	}

	size_type size() const {

		return data.size();
	}

	void insert(key_type key, const value_type& value) {

		data.insert({ key, value });
	}

	size_type erase(key_type frame) {

		return data.erase(frame);
	}

	iterator_type erase(iterator_type it) {

		return data.erase(it);
	}

	iterator_type find(key_type key) {

		return data.find(key);
	}

	bool move(key_type source_key, key_type target_key) {

		auto target_it = data.find(target_key);

		if(target_it == data.end()) {
			auto source_it = data.find(source_key);

			if(source_it != data.end()) {
				value_type value_copy = source_it->second;

				data.erase(source_it);
				data.insert({ target_key, value_copy });

				return true;
			}
		}
		return false;
	}

	iterator_type begin() { return data.begin(); }

	iterator_type end() { return data.end(); }

	const_iterator_type begin() const { return data.cbegin(); }

	const_iterator_type end() const { return data.cend(); }

	reverse_iterator_type rbegin() { return data.rbegin(); }

	reverse_iterator_type rend() { return data.rend(); }

	const_reverse_iterator_type rbegin() const { return data.crbegin(); }

	const_reverse_iterator_type rend() const { return data.crend(); }

	// Returns an iterator pointing to the first element in the container whose key is equivalent or smaller than the given key. If no such element exists, returns end().
	iterator_type lower_bound(key_type key) {

		auto it = data.lower_bound(key);

		if(it == data.end())
			return data.empty() ? data.end() : std::prev(it);
		else if(it->first == key)
			return it;
		else if(it == data.begin())
			return data.end();
		else
			return std::prev(it);
	}

	// Returns an iterator pointing to the first element in the container whose key is greater than the given key. If no such element exists, returns end().
	iterator_type upper_bound(key_type key) {

		return data.upper_bound(key);
	}

	// Returns a pair of iterators pointing to the lower and upper bounds of the given key.
	// The lower bound is defined as the greatest key <= than the given key.
	// The upper bound is defined as the smallest key > than the given key.
	std::pair<iterator_type, iterator_type> bounds(key_type key) {

		return { lower_bound(key), upper_bound(key) };
	}
};
