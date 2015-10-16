#include "color.h"

#ifdef REFLECT_IN_CLASS_NAMESPACE
namespace cgv { namespace media {
#else
namespace cgv { namespace reflect {
#endif

cgv::reflect::enum_reflection_traits<cgv::media::ColorModel> get_reflection_traits(const cgv::media::ColorModel&) 
{
	return cgv::reflect::enum_reflection_traits<cgv::media::ColorModel>("LUM, RGB, HLS, XYZ, YUV, YIQ, Luv, Lab");
}

cgv::reflect::enum_reflection_traits<cgv::media::AlphaModel> get_reflection_traits(const cgv::media::AlphaModel&) 
{
	return cgv::reflect::enum_reflection_traits<cgv::media::AlphaModel>("NO_ALPHA, OPACITY, TRANSPARENCY, EXTINCTION");
}

	}
}
