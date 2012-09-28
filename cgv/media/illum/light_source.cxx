#include "light_source.hh"

namespace cgv { 
	namespace media { 
		namespace illum {

light_source::light_source(LightType _t) 
	: type(_t), local_to_eye_coordinates(true),
	  location(0,1,0,0), ambient(0.1f,0.1f, 0.1f), diffuse(1,1,1), specular(1,1,1),attenuation(1,0,0),spot_direction(0,-1,0),spot_exponent(10),spot_cutoff(45)
{
	if (type != LT_DIRECTIONAL)
		location(3) = 1;
}

		}
	}
}