#pragma once

// C++ STL
#include <iostream>
#include <vector>

// CGV framework core
#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/media/color.h>

// local includes
#include "traj_loader.h"


/// provides read and write capabilites for Hermite splines in .bezdat format
template <class flt_type>
class bezdat_handler : public traj_format_handler<flt_type>
{

public:

	/// real number type
	typedef traj_format_handler::real real;

	/// 2D vector type
	typedef traj_format_handler::Vec2 Vec2;

	/// 3D vector type
	typedef traj_format_handler::Vec3 Vec3;

	/// 4D vector type
	typedef traj_format_handler::Vec4 Vec4;

	/// rgb color type
	typedef traj_format_handler::rgb rgb;


private:

	/// implementation forward
	struct Impl;

	/// implementation handle
	Impl *pimpl;


protected:

	/// explicitely reset implementation-specific state
	virtual void cleanup (void);


public:

	/// default constructor
	bezdat_handler();

	/// the destructor
	virtual ~bezdat_handler();

	/// test if the given data stream appears to be a .bezdat file
	virtual bool can_handle (std::istream &contents) const;

	/// parse the given stream containing the .bezdat file contents and report whether any data was loaded
	virtual bool read (std::istream &contents, unsigned idx_offset=0);

	/// check if the handler currently stores valid loaded data
	virtual bool has_data (void) const;

	/// report the average spatial distance between connected Hermite nodes in the dataset
	virtual real avg_segment_length(void) const;

	/// report the only visual attribute mapping that makes sense with .bezdat
	virtual const visual_attribute_mapping& suggest_mapping (void) const;
};
