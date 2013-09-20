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

#ifdef REFLECT_IN_CLASS_NAMESPACE
}} namespace cgv {	namespace media { namespace illum {
#endif

cgv::reflect::enum_reflection_traits<cgv::media::illum::LightType> get_reflection_traits(const cgv::media::illum::LightType&)
{
	return cgv::reflect::enum_reflection_traits<cgv::media::illum::LightType>("LT_DIRECTIONAL, LT_POINT, LT_SPOT");
}

cgv::reflect::extern_reflection_traits<cgv::media::illum::light_source, cgv::reflect::media::illum::light_source> get_reflection_traits(const cgv::media::illum::light_source&)
{
	return cgv::reflect::extern_reflection_traits<cgv::media::illum::light_source, cgv::reflect::media::illum::light_source>();
}

#ifdef REFLECT_IN_CLASS_NAMESPACE
		}
#endif
	}
}
