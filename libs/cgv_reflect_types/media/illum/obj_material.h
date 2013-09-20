#pragma once

#include <cgv/media/illum/obj_material.hh>
#include <cgv_reflect_types/media/illum/phong_material.h>
#include <cgv/reflect/reflect_extern.h>

#include <cgv_reflect_types/lib_begin.h>

namespace cgv {
	namespace reflect {
		namespace media {
			namespace illum {

struct CGV_API obj_material : public cgv::media::illum::obj_material
{
	bool self_reflect(cgv::reflect::reflection_handler& rh);
};
			}
		}
	}
}

namespace cgv {
	namespace media {
		namespace illum {

extern CGV_API cgv::reflect::extern_reflection_traits<obj_material, cgv::reflect::media::illum::obj_material> get_reflection_traits(const obj_material&);

		}
	}
}

#include <cgv/config/lib_end.h>