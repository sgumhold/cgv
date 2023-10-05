#include "rgbd_calibration.h"
#include <nlohmann/json.hpp>
#include <cgv_json/math.h>
#include <cgv_json/rgbd.h>
#include <fstream>
#include <iomanip>

namespace rgbd {

bool read_rgbd_calibration(const std::string& fn, rgbd::rgbd_calibration& calib, std::string* serial_ptr)
{
	nlohmann::json j;
	std::ifstream is(fn);
	if (is.fail())
		return false;
	is >> j;
	std::string serial;
	j.at("serial").get_to(serial);
	j.at("calib").get_to(calib);
	return true;
}

bool save_rgbd_calibration(const std::string& fn, const rgbd_calibration& calib, const std::string& serial)
{
	nlohmann::json j;
	j["serial"] = serial;
	j["calib"] = calib;
	std::ofstream os(fn);
	if (os.fail())
		return false;
	os << std::setw(4) << j;
	if (os.fail())
		return false;
	return true;
}

bool construct_point(uint16_t x, uint16_t y, uint16_t depth,
	cgv::math::fvec<float, 3>& p,
	const rgbd_calibration& calib,
	const std::vector<cgv::math::fvec<float, 2>>* undistortion_map_ptr,
	double eps,
	unsigned max_nr_iterations,
	double slow_down)
{
	if (depth == 0)
		return false;
	cgv::math::fvec<float, 2> xd;
	if (undistortion_map_ptr) {
		xd = undistortion_map_ptr->at(y * calib.depth.w + x);
		if (xd[0] < -1000.0f)
			return false;
	}
	else {
		cgv::math::camera<double>::distortion_inversion_result dir;
		unsigned iterations = 1;
		cgv::math::fvec<double, 2> xu = calib.depth.pixel_to_image_coordinates(cgv::math::fvec<double, 2>(x, y));
		cgv::math::fvec<double, 2> xd_T = xu;
		dir = calib.depth.invert_distortion_model(xu, xd_T, true, &iterations, eps, max_nr_iterations, slow_down);
		if (dir != cgv::math::distorted_pinhole_types::distortion_inversion_result::convergence)
			return false;
		xd = xd_T;
	}
	float depth_m = float(calib.depth_scale * depth);
	p = cgv::math::fvec<float, 3>(depth_m * xd, depth_m);
	return true;
}

bool lookup_color(const cgv::math::fvec<float, 3>& p,
	cgv::media::color<uint8_t, cgv::media::RGB>& c,
	const frame_type& color_frame,
	const rgbd_calibration& calib)
{
	cgv::math::fvec<double,3> P = (cgv::math::fvec<double, 3>(p) + calib.depth_scale * pose_position(calib.color.pose)) * pose_orientation(calib.color.pose);
	cgv::math::fvec<double,2> xu, xd(P[0] / P[2], P[1] / P[2]);
	auto result = calib.color.apply_distortion_model(xd, xu);
	if (result != cgv::math::distorted_pinhole_types::distortion_result::success)
		return false;
	cgv::math::fvec<double,2> xp = calib.color.image_to_pixel_coordinates(xu);
	if (xp[0] < 0 || xp[1] < 0 || xp[0] >= calib.color.w || xp[1] >= calib.color.h)
		return false;
	uint16_t x = uint16_t(xp[0]);
	uint16_t y = uint16_t(xp[1]);
	const uint8_t* pix_ptr = reinterpret_cast<const uint8_t*>(&color_frame.frame_data[(y * color_frame.width + x) * color_frame.get_nr_bytes_per_pixel()]);
	c = cgv::media::color<uint8_t, cgv::media::RGB>(pix_ptr[2], pix_ptr[1], pix_ptr[0]);
	return true;
}

void construct_point_cloud(
	const frame_type& depth_frame,
	const frame_type& color_or_warped_color_frame,
	std::vector<cgv::math::fvec<float, 3>>& P,
	std::vector<cgv::media::color<uint8_t, cgv::media::RGB>>& C,
	const rgbd_calibration& calib,
	const std::vector<cgv::math::fvec<float, 2>>* undistortion_map_ptr,
	double eps,
	unsigned max_nr_iterations,
	double slow_down)
{
	bool color_is_warped = color_or_warped_color_frame.width == calib.depth.w;
	double sx = 1.0 / depth_frame.width;
	double sy = 1.0 / depth_frame.height;
	for (uint16_t y = 0; y < depth_frame.height; ++y) {
		for (uint16_t x = 0; x < depth_frame.width; ++x) {
			uint16_t depth = reinterpret_cast<const uint16_t&>(depth_frame.frame_data[(y * depth_frame.width + x) * depth_frame.get_nr_bytes_per_pixel()]);
			cgv::math::fvec<float, 3> p;
			cgv::media::color<uint8_t, cgv::media::RGB> c;
			if (color_is_warped) {
				const uint8_t* pix_ptr = reinterpret_cast<const uint8_t*>(&color_or_warped_color_frame.frame_data[(y * depth_frame.width + x) * color_or_warped_color_frame.get_nr_bytes_per_pixel()]);
				c = cgv::media::color<uint8_t, cgv::media::RGB>(pix_ptr[2], pix_ptr[1], pix_ptr[0]);
			}
			if (!construct_point(x, y, depth, p, calib, undistortion_map_ptr, eps, max_nr_iterations, slow_down))
				continue;
			if (!color_is_warped) {
				if (!lookup_color(p, c, color_or_warped_color_frame, calib))
					c = cgv::media::color<uint8_t, cgv::media::RGB>(0, 0, 0);
			}
			P.push_back(p);
			C.push_back(c);
		}
	}
}
void compute_distortion_map(const rgbd_calibration& calib,
	std::vector<cgv::math::fvec<float, 2>>& distortion_map,
	unsigned sub_sample, const cgv::math::fvec<float, 2>& invalid_point,
	double eps, unsigned max_nr_iterations, double slow_down)
{
	calib.depth.compute_distortion_map(distortion_map, sub_sample, invalid_point, eps, max_nr_iterations, slow_down);
}

}

#ifdef REGISTER_SHADER_FILES
#include <cgv/base/register.h>
#include <rgbd_render_shader_inc.h>
#endif
