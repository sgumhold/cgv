#pragma once

#include <cgv/math/fvec.h>
#include <cgv/media/color.h> //@<

#include "lib_begin.h" //@<

namespace cgv { //@<
	namespace media { //@<
		namespace illum { //@<

///@>different light source types
enum LightType { LT_DIRECTIONAL, LT_POINT, LT_SPOT };

///@>simple class to hold the properties of a light source
class CGV_API light_source
{
protected: //@<
	///@>type of light source
	LightType type;
	///@>whether location is relative to eye coordinate system, i.e. model view is identity
	bool local_to_eye_coordinates;
	///@> direction to light source for directional light sources and location of light source for point or spot light sources
	vec3 position;
	///@> radiance for directional light sources and intensity for point light sources
	rgb emission;
	///@> scale to use on emission to retrieve ambient illumination caused by light source
	float ambient_scale;
	///@>constant attenuation coefficients used for positional light sources only
	float constant_attenuation;
	///@>linear attenuation coefficients used for positional light sources only
	float linear_attenuation;
	///@>quadratic attenuation coefficients used for positional light sources only
	float quadratic_attenuation;
	///@>direction of spot light
	vec3 spot_direction;
	///@>spot exponent
	float spot_exponent;
	///@>spot cutoff in degrees
	float spot_cutoff;
public: //@<
	/// init with default values
	light_source(LightType _t = LT_DIRECTIONAL);

	/**name getter and setters for members*/
	///
	void set_type(LightType t) { type = t; }
	///
	LightType get_type() const { return type; }
	///
	LightType& ref_type()      { return type; }

	///
	void set_local_to_eye(bool l) { local_to_eye_coordinates = l; }
	///
	bool is_local_to_eye() const  { return local_to_eye_coordinates; }
	///
	bool& ref_local_to_eye()      { return local_to_eye_coordinates; }

	///
	void set_position(const vec3& loc) { position = loc; }
	///
	const vec3& get_position() const   { return position; }
	///
	vec3& ref_position()   { return position; }

	///
	void set_emission(const rgb& c) { emission = c; }
	///
	const rgb& get_emission() const { return emission; }
	///
	rgb& ref_emission() { return emission; }

	///
	void set_ambient_scale(float c) { ambient_scale = c; }
	///
	float get_ambient_scale() const { return ambient_scale; }
	///
	float& ref_ambient_scale() { return ambient_scale; }

	///
	void set_constant_attenuation(float c) { constant_attenuation = c; }
	///
	float get_constant_attenuation() const { return constant_attenuation; }
	///
	float& ref_constant_attenuation() { return constant_attenuation; }

	///
	void set_linear_attenuation(float c) { linear_attenuation = c; }
	///
	float get_linear_attenuation() const { return linear_attenuation; }
	///
	float& ref_linear_attenuation() { return linear_attenuation; }

	///
	void set_quadratic_attenuation(float c) { quadratic_attenuation = c; }
	///
	float get_quadratic_attenuation() const { return quadratic_attenuation; }
	///
	float& ref_quadratic_attenuation() { return quadratic_attenuation; }

	///
	void set_spot_direction(const vec3& d) { spot_direction = d; }
	///
	const vec3& get_spot_direction() const { return spot_direction; }
	///
	vec3& ref_spot_direction() { return spot_direction; }

	///
	void set_spot_exponent(float e) { spot_exponent = e; }
	///
	float get_spot_exponent() const { return spot_exponent; }
	///
	float& ref_spot_exponent() { return spot_exponent; }

	///
	void set_spot_cutoff(float c) { spot_cutoff = c; }
	///
	float get_spot_cutoff() const { return spot_cutoff; }
	///
	float& ref_spot_cutoff() { return spot_cutoff; }

}; //@<

		} //@<
	} //@<
} //@<

#include <cgv/type/info/type_name.h>

namespace cgv {
	namespace type {
		namespace info {

template <>
struct type_name<cgv::media::illum::light_source>
{
	static const char* get_name() { return "light_source"; }
};
		}
	}
}


#include <cgv/config/lib_end.h>