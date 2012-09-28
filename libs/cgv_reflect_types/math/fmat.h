#pragma	once

#include <cgv/math/fmat.h>
#include "fvec.h"

namespace cgv {
	namespace reflect {
		namespace math {

template <typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
struct fmat : public cgv::math::fmat<T,N,M>
{
	bool self_reflect(cgv::reflect::reflection_handler& rh) {
		return rh.reflect_base(static_cast<cgv::math::fvec<T,N*M>&>(*this));
	}
};
		}

template <typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
extern_string_reflection_traits<cgv::math::fmat<T,N,M>, cgv::reflect::math::fmat<T,N,M> > get_reflection_traits(const cgv::math::fmat<T,N,M>&) { return extern_string_reflection_traits<cgv::math::fmat<T,N,M>, cgv::reflect::math::fmat<T,N,M> >(); }

	}
}
