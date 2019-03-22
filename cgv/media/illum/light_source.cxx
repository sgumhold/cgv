#include "light_source.hh"

namespace cgv { 
	namespace media { 
		namespace illum {

light_source::light_source(LightType _t) 
	: type(_t), local_to_eye_coordinates(true),
	  position(0,1,0), emission(1,1,1), ambient_scale(0.1f), 
	  constant_attenuation(1.0f),linear_attenuation(0.0f),quadratic_attenuation(0.2f),
	  spot_direction(0,-1,0),spot_exponent(10),spot_cutoff(45)
{
}

		}
	}
}