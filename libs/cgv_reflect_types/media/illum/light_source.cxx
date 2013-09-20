#include "light_source.h"
#include <cgv/reflect/set_reflection_handler.h>


namespace cgv {
	namespace reflect {
		namespace media {
			namespace illum {

bool light_source::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return 
		rh.reflect_member("type", type) &&
		rh.reflect_member("local_to_eye_coordinates", local_to_eye_coordinates) &&
		rh.reflect_member("location", location) &&
		rh.reflect_member("ambient", ambient) &&
		rh.reflect_member("diffuse", diffuse) &&
		rh.reflect_member("specular", specular) &&
		rh.reflect_member("attenuation", attenuation) &&
		rh.reflect_member("spot_direction", spot_direction) &&
		rh.reflect_member("spot_exponent", spot_exponent) &&
		rh.reflect_member("spot_cutoff", spot_cutoff);
}

			}
		}

	}
}

namespace cgv {
	namespace media {
		namespace illum {

cgv::reflect::extern_reflection_traits<light_source, cgv::reflect::media::illum::light_source> get_reflection_traits(const light_source&)
{
	return cgv::reflect::extern_reflection_traits<light_source, cgv::reflect::media::illum::light_source>();
}

cgv::reflect::enum_reflection_traits<LightType> get_reflection_traits(const LightType&)
{
	return cgv::reflect::enum_reflection_traits<LightType>("LT_DIRECTIONAL, LT_POINT, LT_SPOT");
}

		}
	}
}
