#pragma once

#include <vector>
#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/media/color.h>
#include <cgv/media/axis_aligned_box.h>

#include "lib_begin.h"

/** define all point cloud relevant types in this helper class */
struct point_cloud_types
{
	/// common type for point, texture und normal coordinates
	typedef float Crd;
	/// 3d point type
	typedef cgv::math::fvec<Crd,3> Pnt;
	/// 3d normal type
	typedef cgv::math::fvec<Crd,3> Nml;
	/// 3d direction type
	typedef cgv::math::fvec<Crd,3> Dir;
	/// 4d homogeneous vector type
	typedef cgv::math::fvec<Crd,4> HVec;
	/// colors are rgb with floating point coordinates
	typedef cgv::media::color<float> Clr;
	/// 3x3 matrix type used for linear transformations
	typedef cgv::math::fmat<Crd,3,3> Mat;
	/// 3x4 matrix type used for affine transformations in reduced homogeneous form
	typedef cgv::math::fmat<Crd,3,4> AMat;
	/// 4x4 matrix type used for perspective transformations in full homogeneous form
	typedef cgv::math::fmat<Crd,4,4> HMat;
	/// type of axis aligned bounding box
	typedef cgv::media::axis_aligned_box<Crd,3> Box;
	/// unsigned integer type used to represent number of points
	typedef cgv::type::uint32_type Cnt;
	/// singed index type used for interation variables
	typedef cgv::type::int32_type Idx;
};

/** simple point cloud data structure with dynamic containers for positions, normals and colors. 
    Normals and colors are optional and can be dynamically allocated and deallocated. */
class CGV_API point_cloud : public point_cloud_types
{	
protected:
	/// container for point positions
	std::vector<Pnt> P;
	/// container for point normals
	std::vector<Nml> N;
	/// container for point colors
	std::vector<Clr> C;

	/// bounding box of all points
	mutable Box B;
private:
	/// flag to remember whether bounding box is out of date and will be recomputed in the get_box() method
	mutable bool box_out_of_date;
protected:
	/// when true, second vector is interpreted as normals when reading an ascii format
	bool no_normals_contained;
	/// flag that tells whether normals are allocated
	bool has_nmls;
	/// flag that tells whether colors are allocated
	bool has_clrs;
	/// read obj-file. Ignores all except of v, vn and vc lines. v lines can be extended by 3 rgb color components
	bool read_obj(const std::string& file_name);
	/// read ascii file with lines of the form x y z r g b I colors and intensity values, where intensity values are ignored
	bool read_xyz(const std::string& file_name);
	/// read ascii file with lines of the form i j x y z I, where ij are pixel coordinates, xyz coordinates and I the intensity
	bool read_pct(const std::string& file_name);
	/// read ascii file with lines of the form x y z [nx ny nz [r g b] ] with optional normals and optional colors (colors only allowed together with normals)
	bool read_points(const std::string& file_name);
	/// read vrml 2.0 files and ignore all but point, normal, and color attributes of a Shape node
	bool read_wrl(const std::string& file_name);
	/// same as read_points but supports files with lines of <x y z r g b> in case that the internal flag no_normals_contained is set before calling read
	bool read_ascii(const std::string& file_name);
	//! read binary format
	/*! Binary format has 8 bytes header encoding two 32-bit unsigned ints n and m.
	    n is the number of points. In case no colors are provided m is the number of normals, i.e. m=0 in case no normals are provided.
		In case colors are present there must be the same number n of colors as points and m is set to 2*n+nr_normals. This is a hack
		resulting from the extension of the format with colors. */
	bool read_bin(const std::string& file_name);
	//! read a ply format.
	/*! Ignores all but the vertex elements and from the vertex elements the properties x,y,z,nx,ny,nz:Float32 and red,green,blue,alpha:Uint8.
	    Colors are transformed to 32-bit floats in the range [0,1] and alpha components are ignored. */
	bool read_ply(const std::string& file_name);
	/// write ascii format, see read_ascii for format description
	bool write_ascii(const std::string& file_name, bool write_nmls = true) const;
	/// write binary format, see read_bin for format description
	bool write_bin(const std::string& file_name) const;
	/// write obj format, see read_obj for format description
	bool write_obj(const std::string& file_name) const;
	/// write ply format, see read_ply for format description
	bool write_ply(const std::string& file_name) const;
public:
	/// construct empty point cloud
	point_cloud();
	/// construct and read file with the read method
	point_cloud(const std::string& file_name);

	/**@name operations */
	//@{
	/// remove all points
	void clear();
	/// append another point cloud
	void append(const point_cloud& pc);
	/// remove all points (including normals and colors) outside of the given box 
	void clip(const Box clip_box);
	/// translate by adding direction vector dir to point positions and update bounding box
	void translate(const Dir& dir);
	/// transform points with linear transform and mark bounding box outdated (careful: normals are not yet transformed!)
	void transform(const Mat& mat);
	/// transform with affine transform and mark bounding box outdated (careful: normals are not yet transformed!)
	void transform(const AMat& amat);
	/// transform with homogeneous transform and w-clip and bounding box outdated (careful: normals are not yet transformed!)
	void transform(const HMat& hmat);
	/// add a point and allocate normal and color if necessary, return index of new point
	Idx add_point(const Pnt& p);
	/// resize the point cloud
	void resize(unsigned nr_points);
	//@}

	/**@name file io*/
	//@{
	//! determine format from extension and read with corresponding read method 
	/*! extension mapping:
	    - read_ascii: *.pnt,*.apc
		- read_bin:   *.bin
		- read_ply:   *.ply
		- read_obj:   *.obj
		- read_points:*.points */
	bool read(const std::string& file_name);
	/// determine format from extension (see read method for extension mapping) and write with corresponding format
	bool write(const std::string& file_name);
	//@}

	/**@name access to geometry*/
	/// return the number of points
	Cnt get_nr_points() const { return (Cnt)P.size(); }
	/// return the i-th point as const reference
	const Pnt& pnt(Idx i) const { return P[i]; }
	/// return the i-th point as reference
	Pnt& pnt(Idx i) { return P[i]; }
	/// return whether the point cloud has normals
	bool has_normals() const;
	/// allocate normals if not already allocated
	void create_normals();
	/// deallocate normals
	void destruct_normals();
	/// return i-th normal as const reference
	const Nml& nml(Idx i) const { return N[i]; }
	/// return i-th normal as reference
	Nml& nml(Idx i) { return N[i]; }
	/// return whether the point cloud has colors
	bool has_colors() const;
	/// allocate colors if not already allocated
	void create_colors();
	/// deallocate colors
	void destruct_colors();
	/// return i-th color as const reference
	const Clr& clr(Idx i) const { return C[i]; }
	/// return i-th color as reference
	Clr& clr(Idx i) { return C[i]; }
	/// return the current bounding box recomputing it in case it is marked as outdated
	const Box& box() const;
	//}
};

#include <cgv/config/lib_end.h>
