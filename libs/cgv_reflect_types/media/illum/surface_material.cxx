#include "surface_material.h"
#include <cgv/reflect/reflection_handler.h>


namespace cgv {
	namespace reflect {
		namespace media {
			namespace illum {

bool surface_material::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return
		rh.reflect_member("brdf_type", brdf_type) &&
		rh.reflect_member("diffuse_reflectance", diffuse_reflectance) &&
		rh.reflect_member("roughness", roughness) &&
		rh.reflect_member("metalness", metalness) &&
		rh.reflect_member("ambient_occlusion", ambient_occlusion) &&
		rh.reflect_member("emission", emission) &&
		rh.reflect_member("transparency", transparency) &&
		rh.reflect_member("propagation_slow_down_re", reinterpret_cast<float(&)[2]>(propagation_slow_down)[0]) &&
		rh.reflect_member("propagation_slow_down_im", reinterpret_cast<float(&)[2]>(propagation_slow_down)[1]) &&
		rh.reflect_member("roughness_anisotropy", roughness_anisotropy) &&
		rh.reflect_member("roughness_orientation", roughness_orientation) &&
		rh.reflect_member("specular_reflectance", specular_reflectance);
}

			}
		}

#ifdef REFLECT_IN_CLASS_NAMESPACE
}} namespace cgv {	namespace media { namespace illum {
#endif

cgv::reflect::enum_reflection_traits<cgv::media::illum::BrdfType> get_reflection_traits(const cgv::media::illum::BrdfType&)
{
	return cgv::reflect::enum_reflection_traits<cgv::media::illum::BrdfType>(
"Lambert,OrenNayar,Strauss,undef,\
Lambert+Phong,OrenNayar+Phong,Strauss+Phong,+Phong,\
Lambert+Blinn,OrenNayar+Blinn,Strauss+Blinn,+Blinn,\
Lambert+CookTorrance,OrenNayar+CookTorrance,Strauss+CookTorrance,+CookTorrance,\
Lambert+Strauss,OrenNayar+Strauss,Strauss+Strauss,+Strauss");
}


cgv::reflect::extern_reflection_traits<cgv::media::illum::surface_material, cgv::reflect::media::illum::surface_material> get_reflection_traits(const cgv::media::illum::surface_material&)
{
	return cgv::reflect::extern_reflection_traits<cgv::media::illum::surface_material, cgv::reflect::media::illum::surface_material>();
}

#ifdef REFLECT_IN_CLASS_NAMESPACE
		}
#endif
	}
}

