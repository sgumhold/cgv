#include "textured_surface_material.h"
#include <cgv/reflect/reflection_handler.h>


namespace cgv {
	namespace reflect {
		namespace media {
			namespace illum {

				bool textured_surface_material::self_reflect(cgv::reflect::reflection_handler& rh)
				{
					return
						rh.reflect_base<surface_material>(*static_cast<surface_material*>(this));
				}

			}
		}

#ifdef REFLECT_IN_CLASS_NAMESPACE
	}
} namespace cgv {
	namespace media {
		namespace illum {
#endif

			cgv::reflect::extern_reflection_traits<cgv::media::illum::surface_material, cgv::reflect::media::illum::textured_surface_material> get_reflection_traits(const cgv::media::illum::textured_surface_material&)
			{
				return cgv::reflect::extern_reflection_traits<cgv::media::illum::surface_material, cgv::reflect::media::illum::textured_surface_material>();
			}

#ifdef REFLECT_IN_CLASS_NAMESPACE
		}
#endif
	}
}

