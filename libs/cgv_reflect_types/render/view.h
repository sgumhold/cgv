#pragma once

#include <cgv/render/view.h>
#include <cgv_reflect_types/math/fvec.h>
#include <cgv/reflect/reflect_extern.h>

#include <cgv_reflect_types/lib_begin.h>

namespace cgv {
	namespace reflect {
		namespace render {

struct CGV_API view : public cgv::render::view
{
	bool self_reflect(cgv::reflect::reflection_handler& rh);
};

		}

#ifdef REFLECT_IN_CLASS_NAMESPACE
}} namespace cgv { namespace render {
#endif

extern CGV_API cgv::reflect::extern_reflection_traits<cgv::render::view, cgv::reflect::render::view> get_reflection_traits(const cgv::render::view&);

	}
}

#include <cgv/config/lib_end.h>
