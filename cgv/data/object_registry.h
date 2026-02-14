#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace cgv {
namespace data {

/// @brief An object registry allows registering and querying objects identified by a unique name.
/// Objects can be retrieved using their name or index. The order of registration defines the order
/// of the objects in the registry.
/// 
/// @tparam T the object type.
template<typename T>
class object_registry {
public:
	using entry_type = std::pair<std::string, T>;
	using const_iterator = typename std::vector<entry_type>::const_iterator;
	using const_reverse_iterator = typename std::vector<entry_type>::const_reverse_iterator;

	size_t size() const {
		return entries_.size();
	}

	bool empty() const {
		return entries_.empty();
	}

	const T& get(size_t index) const {
		return entries_[index].second;
	}

	std::string get_name(size_t index) const {
		return entries_[index].first;
	}

	std::vector<std::string> get_names() const {
		std::vector<std::string> names;
		names.reserve(entries_.size());
		std::transform(entries_.begin(), entries_.end(), std::back_inserter(names), [](const entry_type& entry) { return entry.first; });
		return names;
	}

	bool add(const std::string& name, const T& object) {
		if(index_by_name_.find(name) != index_by_name_.end())
			return false;

		index_by_name_[name] = entries_.size();
		entries_.push_back({ name, object });
		return true;
	}

	void clear() {
		entries_.clear();
		index_by_name_.clear();
	}

	const_iterator begin() const noexcept {
		return entries_.begin();
	}

	const_iterator end() const noexcept {
		return entries_.end();
	}

	const_reverse_iterator rbegin() const noexcept {
		return entries_.rbegin();
	}

	const_reverse_iterator rend() const noexcept {
		return entries_.rend();
	}

	const_iterator cbegin() const noexcept {
		return begin();
	}

	const_iterator cend() const noexcept {
		return end();
	}

	const_reverse_iterator crbegin() const noexcept {
		return rbegin();
	}

	const_reverse_iterator crend() const noexcept {
		return rend();
	}

	const_iterator find(const std::string& name) const {
		auto it = index_by_name_.find(name);
		if(it != index_by_name_.end())
			return entries_.begin() + it->second;
		return entries_.end();
	}

	template<typename UnaryPredicate>
	const_iterator find_first(UnaryPredicate predicate) const {
		for(auto it = entries_.begin(); it != entries_.end(); ++it) {
			if(predicate(it->first, it->second))
				return it;
		}
		return entries_.end();
	}

	template<typename UnaryPredicate>
	std::vector<const_iterator> find_all(UnaryPredicate predicate) const {
		std::vector<const_iterator> result;
		for(auto it = entries_.begin(); it != entries_.end(); ++it) {
			if(predicate(it->first, it->second))
				result.push_back(it);
		}
		return result;
	}

private:
	std::vector<entry_type> entries_;
	std::map<std::string, size_t> index_by_name_;
};

} // namespace data
} // namespace cgv
