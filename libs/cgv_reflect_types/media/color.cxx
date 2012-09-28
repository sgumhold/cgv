#include "color.h"

cgv::reflect::enum_reflection_traits<cgv::media::ColorModel> cgv::reflect::get_reflection_traits(const cgv::media::ColorModel&) 
{
	return enum_reflection_traits<cgv::media::ColorModel>("LUM, RGB, HLS, XYZ, YUV, YIQ, Luv, Lab");
}

cgv::reflect::enum_reflection_traits<cgv::media::AlphaModel> cgv::reflect::get_reflection_traits(const cgv::media::AlphaModel&) 
{
	return enum_reflection_traits<cgv::media::AlphaModel>("NO_ALPHA, OPACITY, TRANSPARENCY, EXTINCTION");
}
