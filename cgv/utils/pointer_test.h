#pragma once

#include <vector>

namespace cgv {
namespace utils {

/* This class provides methods to test if a stored pointer points to addresses of given variables or inside the range of object instances. */
class pointer_test {
private:
	/// Specialization of one_of that takes zero arguments to break recursive variadic template instantiation.
	bool one_of() const {
		return false;
	}

public:
	/// The stored pointer.
	const void* ptr = nullptr;

	/// Instantiate a pointer_test with a const void pointer.
	pointer_test(const void* ptr) : ptr(ptr) {}

	/// Test if the stored pointer points to the given pointer address.
	bool is(const void* ptr) const {
		return this->ptr == ptr;
	}

	/// Test if the stored pointer points to the given variable/object instance.
	template<typename T>
	bool is(const T& ref) const {
		return is(static_cast<const void*>(&ref));
	}

	/// Test if the stored pointer points to one of the given variable/object instances.
	template <typename T, typename... Ts>
	bool one_of(const T& ref, const Ts&... refs) const {

		/* C++ 17
		if constexpr(sizeof...(refs))
			return is(ref) || one_of(refs...);
		else
			return is(ref);
		*/

		return is(ref) || one_of(refs...);
	}

	/// Test if the stored pointer points to one of the given pointer addresses.
	bool one_of(const std::vector<const void*>& ptrs) const {
		for(size_t i = 0; i < ptrs.size(); ++i)
			if(is(ptrs[i])) return true;
		return false;
	}

	/// Test if the stored pointer points inside the address range of the given object instance.
	template <typename T>
	bool member_of(const T& ref) const {
		const void* addr_begin = reinterpret_cast<const void*>(&ref);
		const void* addr_end = reinterpret_cast<const void*>(reinterpret_cast<size_t>(addr_begin) + sizeof(T));
		return ptr >= addr_begin && ptr < addr_end;
	}
};

}
}
