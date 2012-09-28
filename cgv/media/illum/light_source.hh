#pragma once

#include <cgv/math/fvec.h>
#include <cgv/media/color.h> //@<
//#include <cgv/gui/provider.h>

#include "lib_begin.h" //@<

namespace cgv { //@<
	namespace media { //@<
		namespace illum { //@<

///@>different light source types
enum LightType { LT_DIRECTIONAL, LT_POINT, LT_SPOT };

///@>simple class to hold the properties of a light source
class CGV_API light_source
{
public: //@<
	///@>used color type
	typedef color<float,RGB> color_type;
	///@>type of homogeneous locations
	typedef cgv::math::fvec<float,4> hvec_type;
	///@>type of vector used for directions
	typedef cgv::math::fvec<float,3> vec_type;
protected: //@<
	///@>type of light source
	LightType type;
	///@>whether location is relative to eye coordinate system, i.e. model view is identity
	bool local_to_eye_coordinates;
	///@>homogeneous location of light source
	hvec_type location;
	///@>ambient color component
	color_type ambient;
	///@>diffuse color component
	color_type diffuse;
	///@>specular color component
	color_type specular;
	///@>attenuation coefficients used for positional light sources only
	vec_type attenuation;
	///@>direction of spot light
	vec_type spot_direction;
	///@>splot exponent
	float spot_exponent;
	///@>spot cutoff
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
	void set_location(const hvec_type& loc) { location = loc; }
	///
	const hvec_type& get_location() const   { return location; }
	///
	hvec_type& ref_location()   { return location; }

	///
	void set_ambient(const color_type& c) { ambient = c; }
	///
	const color_type& get_ambient() const { return ambient; }
	///
	color_type& ref_ambient() { return ambient; }

	///
	void set_diffuse(const color_type& c) { diffuse = c; }
	///
	const color_type& get_diffuse() const { return diffuse; }
	///
	color_type& ref_diffuse() { return diffuse; }

	///
	void set_specular(const color_type& c) { specular = c; }
	///
	const color_type& get_specular() const { return specular; }
	///
	color_type& ref_specular() { return specular; }

	///
	void set_attenuation(const vec_type& a) { attenuation = a; }
	///
	const vec_type& get_attenuation() const { return attenuation; }
	///
	vec_type& ref_attenuation() { return attenuation; }

	///
	void set_spot_direction(const vec_type& d) { spot_direction = d; }
	///
	const vec_type& get_spot_direction() const { return spot_direction; }
	///
	vec_type& ref_spot_direction() { return spot_direction; }

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