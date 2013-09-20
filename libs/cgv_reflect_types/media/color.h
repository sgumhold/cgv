#pragma once

#include <cgv_reflect_types/math/fvec.h>
#include <cgv/media/color.h>
#include <cgv/reflect/reflect_extern.h>
#include <cgv/reflect/reflect_enum.h>

#include <cgv_reflect_types/lib_begin.h>

namespace cgv {
	namespace reflect {
		namespace media {

template <typename T, cgv::media::ColorModel cm, cgv::media::AlphaModel am> 
struct color : public cgv::media::color<T,cm,am>
{
	bool self_reflect(cgv::reflect::reflection_handler& rh) {
		return
			rh.reflect_member("components", this->components);
	}
};

		}
extern CGV_API enum_reflection_traits<cgv::media::ColorModel> get_reflection_traits(const cgv::media::ColorModel&);
extern CGV_API enum_reflection_traits<cgv::media::AlphaModel> get_reflection_traits(const cgv::media::AlphaModel&);
	}
}

namespace cgv {
	namespace media {
template <typename T, ColorModel cm, AlphaModel am> 
cgv::reflect::extern_string_reflection_traits<color<T,cm,am>, cgv::reflect::media::color<T,cm,am> > get_reflection_traits(const color<T,cm,am>&) { return cgv::reflect::extern_string_reflection_traits<color<T,cm,am>, cgv::reflect::media::color<T,cm,am> >(); }


	}
}

#include <cgv/config/lib_end.h>