#pragma once

/** \file cgv/utils/simple_cache.h
 * Templated helper class to provide simple means of caching key-value pairs in a map.
 */

#include <map>

namespace cgv {
namespace utils {

template<typename K, class V>
class simple_cache {
protected:
	std::map<K, V> data;
	using const_iterator = typename std::map<K, V>::const_iterator;

public:
	simple_cache() {}
	~simple_cache() {}

	void clear() {
		data.clear();
	}

	bool empty() const {
		return data.empty();
	}

	bool valid(const const_iterator& it) const {
		return it != data.end();
	}

	const K key(const const_iterator& it) {
		return it->first;
	}

	const V& value(const const_iterator& it) {
		return it->second;
	}

	const_iterator find(const K& key) const {
		return data.find(key);
	}

	void cache(const K& key, const V& value) {
		data.emplace(key, value);
	}
};

}
}
