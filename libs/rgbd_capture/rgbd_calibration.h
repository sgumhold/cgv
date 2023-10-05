#pragma once

#include <string>
#include "frame.h"
#include <cgv/math/camera.h>
#include <cgv/media/color.h>

#include "lib_begin.h"

namespace rgbd {

	/// calibration information for an rgbd camera
	struct CGV_API rgbd_calibration
	{
		/// scaling of depth camera in meters, i.e. 0.001 corresponds to mm
		double depth_scale = 0.001;
		/// depth camera calibation
		cgv::math::camera<double> depth;
		/// color camera calibation
		cgv::math::camera<double> color;
	};
	/// read calibration from a json file and optionally provide serial string
	extern CGV_API bool read_rgbd_calibration(const std::string& fn, rgbd_calibration& calib, std::string* serial_ptr = 0);
	/// save calibration and serial string to a json file
	extern CGV_API bool save_rgbd_calibration(const std::string& fn, const rgbd_calibration& calib, const std::string& serial);
	/// construct point from pixel coords and depth, given calibration and distortion map or parameters for camera model inversion
	extern CGV_API bool construct_point(uint16_t x, uint16_t y, uint16_t depth,
		cgv::math::fvec<float, 3>& p,
		const rgbd_calibration& calib,
		const std::vector<cgv::math::fvec<float,2>>* undistortion_map_ptr = 0,
		double eps = cgv::math::distortion_inversion_epsilon<double>(),
		unsigned max_nr_iterations = cgv::math::camera<double>::get_standard_max_nr_iterations(),
		double slow_down = cgv::math::camera<double>::get_standard_slow_down());
	/// given a 3D point lookup color from color frame and return whether point projects inside of color frame
	extern CGV_API bool lookup_color(const cgv::math::fvec<float, 3>& p,
		cgv::media::color<uint8_t, cgv::media::RGB>& c,
		const frame_type& color_frame,
		const rgbd_calibration& calib);
	//! construct point cloud from depth and color frame, given calibration and distortion map or parameters for camera model inversion
	/*! color frame is interpreted as warped color frame if it has same width as depth image
	    or as unwarped otherwise.*/
	extern CGV_API void construct_point_cloud(
		const frame_type& depth_frame,
		const frame_type& color_or_warped_color_frame,
		std::vector<cgv::math::fvec<float, 3>>& P,
		std::vector<cgv::media::color<uint8_t, cgv::media::RGB>>& C,
		const rgbd_calibration& calib,
		const std::vector<cgv::math::fvec<float, 2>>* undistortion_map_ptr = 0,
		double eps = cgv::math::distortion_inversion_epsilon<double>(),
		unsigned max_nr_iterations = cgv::math::camera<double>::get_standard_max_nr_iterations(),
		double slow_down = cgv::math::camera<double>::get_standard_slow_down());
	/// compute distortion map from calibration and camera model inversion parameters
	extern CGV_API void compute_distortion_map(const rgbd_calibration& calib,
		std::vector<cgv::math::fvec<float, 2>>& distortion_map,
		unsigned sub_sample = 1,
		const cgv::math::fvec<float, 2>& invalid_point = cgv::math::fvec<float, 2>(-10000.0f),
		double eps = cgv::math::distortion_inversion_epsilon<double>(),
		unsigned max_nr_iterations = cgv::math::camera<double>::get_standard_max_nr_iterations(),
		double slow_down = cgv::math::camera<double>::get_standard_slow_down());
}

#include <cgv/config/lib_end.h>