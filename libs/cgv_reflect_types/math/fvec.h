#pragma once

#include <cgv/math/fvec.h>
#include <cgv/reflect/reflection_handler.h>
#include <cgv/reflect/reflect_extern.h>

namespace cgv {
	namespace reflect {
		namespace math {

template <typename T, cgv::type::uint32_type N>
struct fvec : public cgv::math::fvec<T,N>
{
	bool self_reflect(cgv::reflect::reflection_handler& rh) {
		return rh.reflect_member("coords", this->v);
	}
};
		}

template <typename T, cgv::type::uint32_type N>
extern_string_reflection_traits<cgv::math::fvec<T,N>, cgv::reflect::math::fvec<T,N> > get_reflection_traits(const cgv::math::fvec<T,N>&) { return extern_string_reflection_traits<cgv::math::fvec<T,N>, cgv::reflect::math::fvec<T,N> >(); }

	}
}
