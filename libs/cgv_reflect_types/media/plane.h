#pragma once

#include <cgv/reflect/math/vec.h>
#include <cgv/media/plane.h>
#include <cgv/reflect/reflect_extern.h>

namespace cgv {
	namespace reflect {
		namespace media {

/**
 * An axis aligned box, defined by to points: min and max
 */
template<typename T>
struct plane : public cgv::media::plane<T>
{
	bool self_reflect(cgv::reflect::reflection_handler& rh) {
		return
			rh.reflect_member("h", h);
	}
};

		}

template<typename T>
extern_reflection_traits<cgv::media::plane<T>, cgv::reflect::media::plane<T> > get_reflection_traits(const cgv::media::plane<T>&) { return extern_reflection_traits<cgv::media::plane<T>, cgv::reflect::media::plane<T> >(); }
	}
}

