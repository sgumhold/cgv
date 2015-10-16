#pragma once

#include <cgv/media/illum/phong_material.hh>
#include <cgv_reflect_types/media/color.h>
#include <cgv/reflect/reflect_extern.h>

#include <cgv_reflect_types/lib_begin.h>

namespace cgv {
	namespace reflect {
		namespace media {
			namespace illum {

struct CGV_API phong_material : public cgv::media::illum::phong_material
{
	bool self_reflect(cgv::reflect::reflection_handler& rh);
};
			}
		}

#ifdef REFLECT_IN_CLASS_NAMESPACE
}} namespace cgv {	namespace media { namespace illum {
#endif

extern CGV_API cgv::reflect::extern_reflection_traits<cgv::media::illum::phong_material, cgv::reflect::media::illum::phong_material> get_reflection_traits(const cgv::media::illum::phong_material&);

#ifdef REFLECT_IN_CLASS_NAMESPACE
		}
#endif
	}
}

#include <cgv/config/lib_end.h>