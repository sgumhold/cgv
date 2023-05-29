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

	template <typename T0, typename T1>
	bool one_of(const T0& ref0, const T1& ref1) const {
		return is(ref0) || is(ref1);
	}

	template <typename T0, typename T1, typename T2>
	bool one_of(const T0& ref0, const T1& ref1, const T2& ref2) const {
		return is(ref0) || is(ref1) || is(ref2);
	}

	template <typename T0, typename T1, typename T2, typename T3>
	bool one_of(const T0& ref0, const T1& ref1, const T2& ref2, const T3& ref3) const {
		return is(ref0) || is(ref1) || is(ref2) || is(ref3);
	}

	template <typename T0, typename T1, typename T2, typename T3, typename T4>
	bool one_of(const T0& ref0, const T1& ref1, const T2& ref2, const T3& ref3, const T4& ref4) const {
		return is(ref0) || is(ref1) || is(ref2) || is(ref3) || is(ref4);
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
