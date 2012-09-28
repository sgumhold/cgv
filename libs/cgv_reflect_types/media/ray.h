#pragma once

#include <cgv/reflect/math/vec.h>
#include <cgv/media/ray.h>
#include <cgv/reflect/reflect_extern.h>

namespace cgv {
	namespace reflect {
		namespace media {

/**
 * An axis aligned box, defined by to points: min and max
 */
template<typename T>
struct ray : public cgv::media::ray<T>
{
	bool self_reflect(cgv::reflect::reflection_handler& rh) {
		return
			rh.reflect_member("origin", origin) &&
			rh.reflect_member("direction", direction);
	}
};

		}

template<typename T>
extern_reflection_traits<cgv::media::ray<T>, cgv::reflect::media::ray<T> > get_reflection_traits(const cgv::media::ray<T>&) { return extern_reflection_traits<cgv::media::ray<T>, cgv::reflect::media::ray<T> >(); }

	}
}

