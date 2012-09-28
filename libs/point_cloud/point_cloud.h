#pragma once

#include <vector>
#include <cgv/math/fvec.h>
#include <cgv/media/color.h>
#include <cgv/media/axis_aligned_box.h>

#include "lib_begin.h"

class CGV_API point_cloud
{	
public:
	typedef float coord_type;
	typedef cgv::math::fvec<coord_type,3> Pnt;
	typedef cgv::math::fvec<coord_type,3> Nml;
	typedef cgv::media::color<float> Clr;
	typedef cgv::media::axis_aligned_box<coord_type,3> Box;
	std::vector<Pnt> P;
	std::vector<Nml> N;
	std::vector<Clr> C;
	Box box;
	std::string file_name;
	/// when true, second vector is interpreted as normals when reading an ascii format
	bool no_normals_contained;
public:
	point_cloud();
	point_cloud(const std::string& file_name);
	void clear();
	bool read(const std::string& file_name);
	bool read_obj(const std::string& file_name);
	bool read_points(const std::string& file_name);
	bool read_wrl(const std::string& file_name);
	bool read_ascii(const std::string& file_name);
	bool read_bin(const std::string& file_name);
	const std::string& get_file_name() const { return file_name; }
	void set_file_name(const std::string& _file_name);
	bool write(const std::string& file_name);
	bool write_ascii(bool write_nmls = true) const;
	bool write_bin() const;
	bool write_obj() const;
	void compute_box();
	bool has_colors() const;
	bool has_normals() const;
};

#include <cgv/config/lib_end.h>
