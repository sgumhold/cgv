#pragma once

#include <cgv/math/perm_mat.h>
#include "vec.h"
#include <cgv/reflect/reflection_handler.h>
#include <cgv/reflect/reflect_extern.h>

#include <cgv_reflect_types/lib_begin.h>

namespace cgv {
	namespace reflect {
		namespace math {

struct CGV_API perm_mat : public ::cgv::math::perm_mat
{
	bool self_reflect(cgv::reflect::reflection_handler& rh);
};
		}

extern CGV_API extern_reflection_traits<cgv::math::perm_mat, cgv::reflect::math::perm_mat > get_reflection_traits(const cgv::math::perm_mat&);

	}
}

#include <cgv/config/lib_end.h>