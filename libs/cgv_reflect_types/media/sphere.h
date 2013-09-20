#pragma once

#include <cgv_reflect_types/math/vec.h>
#include <cgv/media/sphere.h>
#include <cgv/reflect/reflect_extern.h>

namespace cgv {
	namespace reflect {
		namespace media {

/**
 * An axis aligned box, defined by to points: min and max
 */
template<typename T, int N>
struct sphere : public cgv::media::sphere<T, N>
{
	bool self_reflect(cgv::reflect::reflection_handler& rh) {
		return
			rh.reflect_member("h", this->h);
	}
};

		}

#ifdef REFLECT_IN_CLASS_NAMESPACE
}} namespace cgv { namespace media {
#endif

template<typename T, int N>
cgv::reflect::extern_reflection_traits<cgv::media::sphere<T,N>, cgv::reflect::media::sphere<T,N> > get_reflection_traits(const cgv::media::sphere<T,N>&) { return cgv::reflect::extern_reflection_traits<cgv::media::sphere<T,N>, cgv::reflect::media::sphere<T,N> >(); }

	}
}

