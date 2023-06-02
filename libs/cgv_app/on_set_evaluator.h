#pragma once

#include <vector>

#include <cgv/render/render_types.h>

namespace cgv {
namespace app {

struct on_set_evaluator {
	const void* member_ptr = nullptr;

	on_set_evaluator(const void* ptr) : member_ptr(ptr) {}

	bool is(const void* ptr) const {
		return member_ptr == ptr;
	}

	template<typename T>
	bool is(const T& ref) const {
		return is(static_cast<const void*>(&ref));
	}

	bool one_of(const std::vector<const void*>& ptrs) const {
		for(size_t i = 0; i < ptrs.size(); ++i)
			if(is(ptrs[i])) return true;
		return false;
	}

	bool one_of() const {
		return false;
	}

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

	template <typename T, cgv::type::uint32_type N>
	bool component_of(const cgv::math::fvec<T, N>& v) const {
		return member_ptr >= v.begin() && member_ptr < v.end();
	}

	template <typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
	bool component_of(const cgv::math::fmat<T, N, M>& m) const {
		return member_ptr >= m.begin() && member_ptr < m.end();
	}

	template <typename T>
	bool member_of(const T* ptr) const {
		const void* addr_begin = reinterpret_cast<const void*>(ptr);
		const void* addr_end = reinterpret_cast<const void*>(reinterpret_cast<size_t>(addr_begin) + sizeof(T));
		return member_ptr >= addr_begin && member_ptr < addr_end;
	}
};

}
}
