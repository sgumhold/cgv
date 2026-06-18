#pragma once

#include <array>
#include <vector>

namespace cgv {
namespace data {

/// @brief This class provides methods to test if a stored pointer points to addresses of given variables or inside the range of object instances.
class informed_ptr {
public:
	informed_ptr(const void* ptr) : ptr_(ptr) {}

	// Get the stored pointer.
	const void* get() const {
		return ptr_;
	}

	/// Return true if the stored pointer is equal to the given pointer.
	template<typename T>
	bool operator==(const T* ptr) const {
		return ptr_ == reinterpret_cast<const void*>(ptr);
	}

	/// Return true if the stored pointer is equal to the given pointer.
	template<typename T>
	bool is(const T* ptr) const {
		return this->operator==(ptr);
	}

	/// Return true if the stored pointer points to the given object.
	template<typename T>
	bool points_to(const T& ref) const {
		return is(reinterpret_cast<const void*>(&ref));
	}

	/// Return true if the stored pointer points to one of the given objects.
	template<typename T, typename... Ts>
	bool points_to_one_of(const T& ref, const Ts&... refs) const {

		/* C++ 17
		if constexpr(sizeof...(refs))
			return is(ref) || one_of(refs...);
		else
			return is(ref);
		*/

		return points_to(ref) || points_to_one_of(refs...);
	}

	/// Return true if the stored pointer points inside the address range of the given object.
	template<typename T>
	bool points_to_member_of(const T& ref) const {
		return is_in_counted_range(&ref, sizeof(T));
	}

	/// Return true if the stored pointer points to an element inside the given c-style array.
	template<typename T>
	bool points_inside(const T* array, size_t n) const {
		return is_in_counted_range(array, sizeof(T) * n);
	}

	/// Return true if the stored pointer points to an element of the data of the given std::array.
	template<typename T, size_t N>
	bool points_to_data_of(const std::array<T, N>& array) const {
		return is_in_counted_range(array.data(), sizeof(T) * N);
	}

	/// Return true if the stored pointer points to an element of the data of the given std::vector.
	template<typename T>
	bool points_to_data_of(const std::vector<T>& vector) const {
		return is_in_counted_range(vector.data(), sizeof(T) * vector.size());
	}

	/// Return true if the stored pointer is inside the range [first, last).
	template<typename T>
	bool is_in_range(const T* first, const T* last) const {
		return ptr_ >= reinterpret_cast<const void*>(first) && ptr_ < reinterpret_cast<const void*>(last);
	}

	/// Return iterator to the element in vector that the stored pointer points to or vector.end();
	template<typename T>
	typename std::vector<T>::iterator find_in_data_of(std::vector<T>& vector) const {
		if(points_to_data_of(vector)) {
			for(auto it = vector.begin(); it != vector.end(); ++it) {
				if(is(&*it))
					return it;
			}
		}
		return vector.end();
	}

	/// Return const iterator to the element in vector that the stored pointer points to or vector.end();
	template<typename T>
	typename std::vector<T>::const_iterator find_in_data_of(const std::vector<T>& vector) const {
		return find_in_data_of(const_cast<std::vector<T>&>(vector));
	}

private:
	/// Specialization of points_to_one_of that takes zero arguments to break recursive variadic template instantiation.
	bool points_to_one_of() const {
		return false;
	}

	/// Return true if the stored pointer is inside the range [ptr, ptr + bytes).
	template<typename T>
	bool is_in_counted_range(const T* ptr, size_t bytes) const {
		const void* first = reinterpret_cast<const void*>(ptr);
		const void* last = reinterpret_cast<const void*>(reinterpret_cast<size_t>(first) + bytes);
		return ptr_ >= first && ptr_ < last;
	}

	/// The stored pointer.
	const void* ptr_ = nullptr;
};

} // namespace data
} // namespace cgv
