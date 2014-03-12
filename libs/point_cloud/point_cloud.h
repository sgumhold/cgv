#pragma once

#include <vector>
#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/media/color.h>
#include <cgv/media/axis_aligned_box.h>

#include "lib_begin.h"

struct point_cloud_types
{
	typedef float Crd;
	typedef cgv::math::fvec<Crd,3> Pnt;
	typedef cgv::math::fvec<Crd,3> Nml;
	typedef cgv::math::fvec<Crd,3> Dir;
	typedef cgv::math::fvec<Crd,4> HVec;
	typedef cgv::media::color<float> Clr;
	typedef cgv::math::fmat<Crd,3,3> Mat;
	typedef cgv::math::fmat<Crd,3,4> AMat;
	typedef cgv::math::fmat<Crd,4,4> HMat;
	typedef cgv::media::axis_aligned_box<Crd,3> Box;
	typedef unsigned Cnt;
	typedef int Idx;
};

class CGV_API point_cloud : public point_cloud_types
{	
protected:
	std::vector<Pnt> P;
	std::vector<Nml> N;
	std::vector<Clr> C;

	mutable Box B;
	mutable bool box_out_of_date;
	/// when true, second vector is interpreted as normals when reading an ascii format
	bool no_normals_contained;

	bool has_nmls;
	bool has_clrs;

	bool read_obj(const std::string& file_name);
	bool read_points(const std::string& file_name);
	bool read_wrl(const std::string& file_name);
	bool read_ascii(const std::string& file_name);
	bool read_bin(const std::string& file_name);
	bool read_ply(const std::string& file_name);
	bool write_ascii(const std::string& file_name, bool write_nmls = true) const;
	bool write_bin(const std::string& file_name) const;
	bool write_obj(const std::string& file_name) const;
	bool write_ply(const std::string& file_name) const;
public:
	/// construct empty point cloud
	point_cloud();
	/// construct and read file
	point_cloud(const std::string& file_name);
	/**@name operations */
	//@{
	/// remove all points
	void clear();
	/// append another point cloud
	void append(const point_cloud& pc);
	/// clip on box
	void clip(const Box clip_box);
	/// translate by direction
	void translate(const Dir& dir);
	/// transform with linear transform 
	void transform(const Mat& mat);
	/// transform with affine transform 
	void transform(const AMat& amat);
	/// transform with homogeneous transform and w-clip
	void transform(const HMat& hmat);
	/// add a point and allocate normal and color if necessary
	Idx add_point(const Pnt& p);
	//@}

	/**@name io*/
	//@{
	/// read and determine format from extension
	bool read(const std::string& file_name);
	/// write and determine format from extension
	bool write(const std::string& file_name);
	//@}

	/**@name access to geometry*/
	///
	Cnt get_nr_points() const { return (Cnt)P.size(); }
	///
	const Pnt& pnt(Idx i) const { return P[i]; }
	///
	Pnt& pnt(Idx i) { return P[i]; }
	///
	bool has_normals() const;
	///
	void create_normals();
	///
	void destruct_normals();
	///
	const Nml& nml(Idx i) const { return N[i]; }
	///
	Nml& nml(Idx i) { return N[i]; }
	///
	bool has_colors() const;
	///
	void create_colors();
	///
	void destruct_colors();
	///
	const Clr& clr(Idx i) const { return C[i]; }
	///
	Clr& clr(Idx i) { return C[i]; }
	/// return box
	const Box& box() const;
	//}
};

#include <cgv/config/lib_end.h>
