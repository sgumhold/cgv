#pragma once

// C++ STL
#include <iostream>
#include <vector>

// CGV framework core
#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/media/color.h>

// local includes
//#include "trajectory_loader.h"


/// provides read and write capabilites for Hermite splines in .bezdat format
template <class flt_type>
class bezdat_handler
{

public:

	/// real number type
	typedef flt_type real;

	/// 2D vector type
	typedef cgv::math::fvec<real, 2> Vec2;

	/// 3D vector type
	typedef cgv::math::fvec<real, 3> Vec3;

	/// 4D vector type
	typedef cgv::math::fvec<real, 4> Vec4;

	/// 4x4 matrix type
	typedef cgv::math::fmat<real, 4, 4> Mat4;

	// color type of .bezdat color attribute
	typedef cgv::media::color<float, cgv::media::RGB, cgv::media::NO_ALPHA> rgb;


private:

	/// implementation forward
	struct Impl;

	/// implementation handle
	Impl *pimpl;


public:

	/// default constructor
	bezdat_handler();

	/// virtual base destructor - causes vtable creation
	virtual ~bezdat_handler();

	/// Parse the given stream containing the .bezdat file contents and report whether any data was loaded.
	/// Retrieve the loaded data using the attribute accessors.
	bool read (std::istream &contents);

	/// check if the handler currently stores valid loaded data
	bool has_data (void) const;

	/// access positions
	const std::vector<Vec3>& positions (void) const;

	/// access tangents (including radius derivatives in w-component)
	const std::vector<Vec4>& tangents (void) const;

	/// access radii
	const std::vector<real>& radii (void) const;

	/// access colors
	const std::vector<rgb>& colors (void) const;

	/// access indices
	const std::vector<unsigned>& indices (void) const;
};


/// converts 255-based 3-vectors to 1-based float RGB colors
template <class vec_type>
typename bezdat_handler<float>::rgb vec3_to_rgb (const vec_type &v)
{
	return bezdat_handler<float>::rgb(float(v.x())/255.f, float(v.y())/255.f, float(v.z())/255.f);
}

/// constructs a 4-vector from a 3-vector and a scalar
template <class vec3_type, class scalar_type>
cgv::math::fvec<typename vec3_type::value_type, 4> vec4_from_vec3s (const vec3_type& v, scalar_type s)
{
	return cgv::math::fvec<typename vec3_type::value_type, 4>(v.x(), v.y(), v.z(), vec3_type::value_type(s));
}
