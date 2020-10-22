#pragma once

#include <cgv/math/fvec.h>
#include <cgv/data/data_view.h>
#include <deque>

#include "lib_begin.h"

namespace rgbd {

class CGV_API rgbd_mouse
{
public:
	typedef cgv::math::fvec<float,3> vec3;

protected:
	vec3 last_pos;

	vec3 p_min, p_max;
	float z_click, pitch;

	std::deque<vec3> last_poses;
	std::deque<unsigned short*> last_images;
	std::deque<float> last_weights;
	unsigned last_pos_idx;
	unsigned nr_pred;
	unsigned nr_cached, nr_cached_images;
	float inlier_radius, weight_exp;
	float f;

	int reduction_factor;
	cgv::data::data_format reduced_df;
	cgv::data::data_view reduced_data;
	cgv::data::data_view* data_ptr;

	int nr_patches_w, nr_patches_h;

	void reduce(const cgv::data::const_data_view& depth_data);
	vec3 track_patch(int x, int y, unsigned nr_patches_w, unsigned nr_patches_h) const;
	///
	vec3 track_rectangle(int x0, int y0, int x1, int y1, int d0, int d1) const;
	///
	void filter_tracked_pos();
	void filter_tracked_pos_linear();
	///
	void filter_depth_image(cgv::data::data_view& depth_data);
public:
	float& ref_inlier_radius() { return inlier_radius; }
	unsigned& ref_nr_cached() { return nr_cached; }
	float& ref_weight_exp() { return weight_exp; }
	///
	rgbd_mouse();
	///
	void set_config(const vec3& p_min, const vec3& p_max, float z_click, float pitch);
	///
	bool save_config(const std::string& file_name) const;
	///
	bool load_config(const std::string& file_name);
	///
	float get_pitch() const { return pitch; }
	///
	float get_z_click() const { return z_click; }
	///
	const vec3& get_p_min() const { return p_min; }
	///
	const vec3& get_p_max() const { return p_max; }
	///
	vec3 track(cgv::data::data_view& depth_data);
	///
//	vec3 track_bilateral(const cgv::data::const_data_view& depth_data);
	///
//	void track_all(const cgv::data::const_data_view& depth_data, std::vector<vec3>& vec3s);
};

}
#include <cgv/config/lib_end.h>