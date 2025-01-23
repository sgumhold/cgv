#pragma once

#include <cgv/math/fvec.h>
#include <cgv/media/axis_aligned_box.h>
#include <vector>

@#include "reflection_test.h"

#include "lib_begin.h" // @<

/// @>
enum ExternalEnum
{
	AA,BB,CC,DD
};

namespace test { // @<
	/// @>
	enum AnotherEnum
	{
		A,B,C
	};
	namespace cgv { // @<



/** drawable that visualizes implicit surfaces by contouring them with marching cubes or
    dual contouring. @> */
class CGV_API my_class : public my_base_class, public std::vector<double>
{
public: // @<
	/// type of axis aligned box used to define the tesselation domain @>
	typedef cgv::media::axis_aligned_box<double,3> box_type;
	/// point type @>
	typedef cgv::math::fvec<double,3> pnt_type;
	/// type of contouring method @>
	enum ContouringType { MARCHING_CUBES, DUAL_CONTOURING };
	/// normal computation type @>
	enum NormalComputationType { GRADIENT_NORMALS, FACE_NORMALS, CORNER_NORMALS, CORNER_GRADIENTS };
	/// info of marching cube @>
	struct MarchingInfo 
	{
		/// @>
		typedef double coord_type;
		/// @>
		enum Number { ONE = 1, TWO, FOUR = 4 };
		/// @>
		ContouringType ct;
		/// @>
		NormalComputationType nct;
		/// @>
		bool shown;
		/// @>
		Number nbr;
		/// @>
		ExternalEnum ee;
		/// @>
		AnotherEnum ae;
		/// @>
		DummyEnum de;
	}; // @<
protected: // @<
	//@>
	MarchingInfo info;
	//@>
	unsigned int res;
	//@>
	box_type box;
	//@>
	pnt_type pnt;
	//@>
	ContouringType contouring_type;
	//@>
	double normal_threshold;
	//@>
	double consistency_threshold;
	//@>
	unsigned int max_nr_iters;
	//@>
	NormalComputationType normal_computation_type;
	//@>
	unsigned int ix;
	//@>
	unsigned int iy;
	//@>
	unsigned int iz;
	//@>
	bool wireframe;
	//@>
	bool show_sampling_grid;
	//@>
	bool show_sampling_locations;
	//@>
	bool show_box;
	//@>
	bool show_mini_box;
	//@>
	bool show_gradient_normals;
	//@>
	bool show_mesh_normals;

	//@>
	double epsilon;
	//@>
	double grid_epsilon;

	//@>
	int nr_faces;
	//@>
	int nr_vertices;
public: // @<
}; // @<

/// ref counted pointer to implicit drawable @>
typedef cgv::data::ref_ptr<my_class> my_class_ptr;

	} // @<
} // @<

#include <cgv/config/lib_end.h> // @<