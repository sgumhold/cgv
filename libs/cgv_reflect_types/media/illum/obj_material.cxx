#include "obj_material.h"
#include <cgv/reflect/set_reflection_handler.h>


namespace cgv {
	namespace reflect {
		namespace media {
			namespace illum {

bool obj_material::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return 
		rh.reflect_base(static_cast<cgv::media::illum::phong_material&>(*this)) &&
		rh.reflect_member("name", name) &&
		rh.reflect_member("opacity", opacity) &&
		rh.reflect_member("ambient_texture_name", ambient_texture_name) &&
		rh.reflect_member("diffuse_texture_name", diffuse_texture_name) &&
		rh.reflect_member("specular_texture_name", specular_texture_name) &&
		rh.reflect_member("emission_texture_name", emission_texture_name) &&
		rh.reflect_member("bump_scale", bump_scale) &&
		rh.reflect_member("bump_texture_name", bump_texture_name);
}

			}
		}
	}
}

namespace cgv {
	namespace media {
		namespace illum {


cgv::reflect::extern_reflection_traits<obj_material, cgv::reflect::media::illum::obj_material> get_reflection_traits(const obj_material&)
{
	return cgv::reflect::extern_reflection_traits<obj_material, cgv::reflect::media::illum::obj_material>();
}

		}
	}
}

