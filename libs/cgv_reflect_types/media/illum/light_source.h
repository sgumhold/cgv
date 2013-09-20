#pragma once

#include <cgv/media/illum/light_source.hh>
#include <cgv_reflect_types/media/color.h>
#include <cgv_reflect_types/math/fvec.h>
#include <cgv/reflect/reflect_extern.h>

#include <cgv_reflect_types/lib_begin.h>

namespace cgv {
	namespace reflect {
		namespace media {
			namespace illum {

struct CGV_API light_source : public cgv::media::illum::light_source
{
	bool self_reflect(cgv::reflect::reflection_handler& rh);
};
			}
		}


#ifdef REFLECT_IN_CLASS_NAMESPACE
}} namespace cgv {	namespace media { namespace illum {
#endif

extern CGV_API cgv::reflect::enum_reflection_traits<cgv::media::illum::LightType> get_reflection_traits(const cgv::media::illum::LightType&);

extern CGV_API cgv::reflect::extern_reflection_traits<cgv::media::illum::light_source, cgv::reflect::media::illum::light_source> get_reflection_traits(const cgv::media::illum::light_source&);

#ifdef REFLECT_IN_CLASS_NAMESPACE
		}
#endif
	}
}

#include <cgv/config/lib_end.h>
