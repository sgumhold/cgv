#pragma once

#include <vector>
#include <cgv/utils/statistics.h>
#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/math/quaternion.h>
#include <cgv/media/color.h>
#include <cgv/media/axis_aligned_box.h>

#include "lib_begin.h"

#define BYTE_COLORS

/** define all point cloud relevant types in this helper class */
struct point_cloud_types
{
	/// common type for point, texture und normal coordinates
	typedef float Crd;
#ifdef BYTE_COLORS
	/// type of color components
	typedef cgv::type::uint8_type ClrComp;
	static ClrComp byte_to_color_component(cgv::type::uint8_type c) { return c; }
	static ClrComp float_to_color_component(double c) { return cgv::type::uint8_type(255 * c); }
	static cgv::type::uint8_type color_component_to_byte(ClrComp c) { return c; }
	static float color_component_to_float(ClrComp c) { return 1.0f/255 * c; }
#else
	/// type of color components
	typedef float ClrComp;
	static ClrComp byte_to_color_component(cgv::type::uint8_type c) { return c*1.0f/255; }
	static ClrComp float_to_color_component(double c) { return float(c); }
	static cgv::type::uint8_type color_component_to_byte(ClrComp c) { return cgv::type::uint8_type(255*c); }
	static float color_component_to_float(ClrComp c) { return c; }
#endif // BYTE_COLORS
	/// floating point color type
	typedef cgv::media::color<float, cgv::media::RGB, cgv::media::OPACITY> RGBA;
	/// 3d point type
	typedef cgv::math::fvec<Crd,3> Pnt;
	/// 3d normal type
	typedef cgv::math::fvec<Crd,3> Nml;
	/// 3d direction type
	typedef cgv::math::fvec<Crd, 3> Dir;
	/// 2d texture coordinate type
	typedef cgv::math::fvec<Crd, 2> TexCrd;
	/// 4d homogeneous vector type
	typedef cgv::math::fvec<Crd,4> HVec;
	/// colors are rgb with floating point coordinates
	typedef cgv::media::color<ClrComp> Clr;
	/// rgba colors used for components
	typedef cgv::media::color<ClrComp,cgv::media::RGB,cgv::media::OPACITY> Rgba;
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
	/// 2d pixel position type
	typedef cgv::math::fvec<Idx, 2> PixCrd;
	/// type of pixel coordinate range
	typedef cgv::media::axis_aligned_box<Idx, 2> PixRng;
	/// type of texture coordinate box
	typedef cgv::media::axis_aligned_box<Crd, 2> TexBox;
	/// quaternions used to represent rotations
	typedef cgv::math::quaternion<Crd> Qat;
	/// simple structure to store the point range of a point cloud component
	struct component_info
	{
		std::string name;
		size_t index_of_first_point;
		size_t nr_points;
		component_info(size_t _first = 0, size_t _nr = 0) : index_of_first_point(_first), nr_points(_nr) {}
	};
};

class CGV_API index_image : public point_cloud_types
{
	Idx width;
	PixRng pixel_range;
	std::vector<Idx> indices;
	Idx get_index(const PixCrd& pixcrd) const;
public:
	/// construct empty index image
	index_image();
	/// return pixel range of image
	const PixRng& get_pixel_range() const  { return pixel_range;  }
	/// 
	static PixCrd image_neighbor_offset(int i);
	/// return image width
	Idx get_width() const { return width;  }
	/// return image height
	Idx get_height() const { return pixel_range.get_extent()(1); }
	/// construct image of given dimensions and initialize indices to given value
	void create(const PixRng& _pixel_range, Idx _initial_value = -1);
	/// read access pixel index through pixel coordinates
	Idx operator () (const PixCrd& pixcrd) const;
	/// write access pixel index through pixel coordinates
	Idx& operator () (const PixCrd& pixcrd);
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
	/// container for point texture coordinates 
	std::vector<TexCrd> T;
	/// container for point pixel coordinates 
	std::vector<PixCrd> I;

	/// container to store  one component index per point
	std::vector<unsigned> component_indices;

	/// container to store point range per component
	std::vector<component_info> components;
	/// return begin point index for iteration
	Idx begin_index(Idx component_index) const;
	/// return end point index for iteration
	Idx end_index(Idx component_index) const;
	/// container storing component colors
	std::vector<RGBA> component_colors;
	/// container storing component rotationa
	std::vector<Qat> component_rotations;
	/// container storing component translations
	std::vector<Dir> component_translations;

	/// container storing component bounding boxes
	mutable std::vector<Box> component_boxes;
	/// container storing component pixel ranges
	mutable std::vector<PixRng> component_pixel_ranges;

	/// bounding box of all points
	mutable Box B;
	/// range of pixel coordinates
	mutable PixRng PR;
	///
	friend class point_cloud_interactable;
	friend class point_cloud_viewer;
	friend class gl_point_cloud_drawable;
	friend class vr_rgbd;
private:
	mutable std::vector<bool> comp_box_out_of_date;
	mutable std::vector<bool> comp_pixrng_out_of_date;
	/// flag to remember whether bounding box is out of date and will be recomputed in the box() method
	mutable bool box_out_of_date;
	/// flag to remember whether pixel coordinate range is out of date and will be recomputed in the pixel_range() method
	mutable bool pixel_range_out_of_date;
protected:
	/// when true, second vector is interpreted as normals when reading an ascii format
	bool no_normals_contained;
	/// flag that tells whether normals are allocated
	bool has_nmls;
	/// flag that tells whether colors are allocated
	bool has_clrs;
	/// flag that tells whether texture coordinates are allocated
	bool has_texcrds;
	/// flag that tells whether pixel coordinates are allocated
	bool has_pixcrds;
	/// flag that tells whether components are allocated
	bool has_comps;
	/// flag that tells whether component transformations are allocated
	bool has_comp_trans;
	/// flag that tells whether component colors are allocated
	bool has_comp_clrs;
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
	/// read txt file from leica scanner
	bool read_txt(const std::string& file_name);
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
	/// permute points
	void permute(std::vector<Idx>& perm, bool permute_component_indices);
	/// translate by adding direction vector dir to point positions and update bounding box
	void translate(const Dir& dir, Idx component_index = -1);
	/// rotate points and normals with quaternion
	void rotate(const Qat& qat, Idx component_index = -1);
	/// transform points with linear transform and mark bounding box outdated (careful: normals are not yet transformed!)
	void transform(const Mat& mat);
	/// transform with affine transform and mark bounding box outdated (careful: normals are not yet transformed!)
	void transform(const AMat& amat);
	/// transform with homogeneous transform and w-clip and bounding box outdated (careful: normals are not yet transformed!)
	void transform(const HMat& hmat);
	/// add a point and allocate normal and color if necessary, return index of new point
	size_t add_point(const Pnt& p);
	/// add a point and a normal, add a color if necessary, return index of new point
	size_t add_point(const Pnt& p, const Nml& n);
	/// add a point and a color, add a normal if necessary, return index of new point
	size_t add_point(const Pnt& p, const Clr& c);
	/// add a point, a normal and a color, return index of new point
	size_t add_point(const Pnt& p, const Nml& n, const Clr& c);
	/// resize the point cloud
	void resize(size_t nr_points);
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
	/// read component transformations from ascii file with 12 numbers per line (9 for rotation matrix and 3 for translation vector)
	bool read_component_transformations(const std::string& file_name);
	/// determine format from extension (see read method for extension mapping) and write with corresponding format
	bool write(const std::string& file_name);
	/// write component transformations to ascii file with 12 numbers per line (9 for rotation matrix and 3 for translation vector)
	bool write_component_transformations(const std::string& file_name, bool as_matrices = true) const;
	//@}

	/**@name access to geometry*/
	/// return the number of points
	Cnt get_nr_points() const { return (Cnt)P.size(); }
	/// return the i-th point as const reference
	const Pnt& pnt(size_t i) const { return P[i]; }
	/// return the i-th point as reference
	Pnt& pnt(size_t i) { return P[i]; }
	/// return the i_th point, in case components and component transformations are created, transform point with its compontent's transformation before returning it 
	Pnt transformed_pnt(size_t i) const;

	/// return whether the point cloud has normals
	bool has_normals() const;
	/// allocate normals if not already allocated
	void create_normals();
	/// deallocate normals
	void destruct_normals();
	/// return i-th normal as const reference
	const Nml& nml(size_t i) const { return N[i]; }
	/// return i-th normal as reference
	Nml& nml(size_t i) { return N[i]; }

	/// return whether the point cloud has colors
	bool has_colors() const;
	/// allocate colors if not already allocated
	void create_colors();
	/// deallocate colors
	void destruct_colors();
	/// return i-th color as const reference
	const Clr& clr(size_t i) const { return C[i]; }
	/// return i-th color as reference
	Clr& clr(size_t i) { return C[i]; }

	/// return whether the point cloud has texture coordinates
	bool has_texture_coordinates() const;
	/// allocate texture coordinates if not already allocated
	void create_texture_coordinates();
	/// deallocate texture coordinates
	void destruct_texture_coordinates();
	/// return i-th texture coordinate as const reference
	const TexCrd& texcrd(size_t i) const { return T[i]; }
	/// return i-th texture coordinate as reference
	TexCrd& texcrd(size_t i) { return T[i]; }

	/// return whether the point cloud has pixel coordinates
	bool has_pixel_coordinates() const;
	/// allocate pixel coordinates if not already allocated
	void create_pixel_coordinates();
	/// deallocate pixel coordinates
	void destruct_pixel_coordinates();
	/// return i-th pixel coordinate as const reference
	const PixCrd& pixcrd(size_t i) const { return I[i]; }
	/// return i-th pixel coordinate as reference
	PixCrd& pixcrd(size_t i) { return I[i]; }

	/// return number of components
	size_t get_nr_components() const;
	/// add a new component
	Idx add_component();
	/// return whether the point cloud has component indices and point ranges
	bool has_components() const;
	/// allocate component indices and point ranges if not already allocated
	void create_components();
	/// remove all points from the given component
	void clear_component(size_t i);
	/// deallocate component indices and point ranges
	void destruct_components();
	/// return i-th component index as const reference
	unsigned component_index(size_t i) const { return component_indices[i]; }
	/// return i-th component index as reference
	unsigned& component_index(size_t i) { return component_indices[i]; }
	/// return the point range of a component as const reference
	const component_info& component_point_range(Idx ci) const { return components[ci]; }
	/// return the point range of a component as reference
	component_info& component_point_range(Idx ci) { return components[ci]; }
	/// return name of i-th component
	const std::string& component_name(Idx ci) const { return components[ci].name; }
	/// return name of i-th component
	std::string& component_name(Idx ci) { return components[ci].name; }
	/// return whether the point cloud has component colors
	bool has_component_colors() const;
	/// allocate component colors if not already allocated
	void create_component_colors();
	/// deallocate colors
	void destruct_component_colors();
	/// return ci-th component colors as const reference
	const RGBA& component_color(Idx ci) const { return component_colors[ci]; }
	/// return ci-th component color as reference
	RGBA& component_color(Idx ci) { return component_colors[ci]; }

	/// return whether the point cloud has component tranformations
	bool has_component_transformations() const;
	/// allocate component tranformations if not already allocated
	void create_component_tranformations();
	/// deallocate tranformations
	void destruct_component_tranformations();
	/// return ci-th component rotation as const reference
	const Qat& component_rotation(Idx ci) const { return component_rotations[ci]; }
	/// return ci-th component rotation as reference
	Qat& component_rotation(Idx ci) { return component_rotations[ci]; }
	/// return ci-th component translation as const reference
	const Dir& component_translation(Idx ci) const { return component_translations[ci]; }
	/// return ci-th component translation as reference
	Dir& component_translation(Idx ci) { return component_translations[ci]; }
	/// apply transformation of given component (or all of component index is -1) to influenced points
	void apply_component_transformation(Idx component_index = -1);
	/// set the component transformation of given component (or all of component index is -1) to identity
	void reset_component_transformation(Idx component_index = -1);

	/// return the current bounding box of a component (or the whole point cloud if given component_index is -1)
	const Box& box(Idx component_index = -1) const;
	/// return the range of the stored pixel coordinates of a component (or the whole point cloud if given component_index is -1)
	const PixRng& pixel_range(Idx component_index = -1) const;
	/// compute an image with a point index stored per pixel, store indices in the pixel range of the point cloud with a border of the given size
	void compute_index_image(index_image& img, unsigned border_size = 0, Idx component_index = -1);
	/// detect outliers based on neighborhood in pixel coordinates
	void detect_outliers(const index_image& img, std::vector<size_t>& outliers) const;
	/// compute the range of direct neighbor distances
	void compute_image_neighbor_distance_statistic(const index_image& img, cgv::utils::statistics& distance_stats, Idx component_idx = -1);
	/// collect the indices of the neighbor points of point pi
	Cnt collect_valid_image_neighbors(size_t pi, const index_image& img, std::vector<size_t>& Ni, Crd distance_threshold = 0.0f) const;
	/// compute the normals with the help of pixel coordinates
	void estimate_normals(const index_image& img, Crd distance_threshold = 0.0f, Idx component_idx = -1, int* nr_isolated = 0, int* nr_iterations = 0, int* nr_left_over = 0);
	//}
};

#include <cgv/config/lib_end.h>
