#pragma once

#include <cgv/defines/join.h>

namespace cgv {
	namespace defines {

template <bool x> struct STATIC_ASSERTION_FAILURE;
template <> struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };
template<int x> struct static_assert_test{};

	}
}

#define CGV_DEFINES_ASSERT( ... ) \
	typedef ::cgv::defines::static_assert_test<\
      sizeof(::cgv::defines::STATIC_ASSERTION_FAILURE< (bool)( __VA_ARGS__ ) >)>\
         CGV_DEFINES_JOIN(_static_assert_test_, __LINE__);

