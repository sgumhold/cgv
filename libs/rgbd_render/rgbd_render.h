#pragma once

#include <cgv/math/camera.h>
#include <cgv/render/context.h>
#include <cgv/render/texture.h>
#include <cgv/render/shader_program.h>
#include <rgbd_capture/frame.h>
#include <rgbd_capture/rgbd_device.h>
#include <string>

#include "lib_begin.h"

namespace rgbd {
	extern CGV_API bool create_or_update_texture_from_frame(cgv::render::context& ctx,
		cgv::render::texture& tex, const rgbd::frame_type& frame);
	extern CGV_API void construct_rgbd_render_data(
		const rgbd::frame_type& depth_frame,
		std::vector<cgv::usvec3>& sP,
		uint16_t sub_sample = 1, uint16_t sub_line_sample = 1);
	extern CGV_API void construct_rgbd_render_data_with_color(
		const rgbd::frame_type& depth_frame,
		const rgbd::frame_type& warped_color_frame,
		std::vector<cgv::usvec3>& sP,
		std::vector<cgv::rgb8>& sC,
		uint16_t sub_sample = 1, uint16_t sub_line_sample = 1);
	extern CGV_API void set_camera_calibration_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, const std::string& name, const cgv::math::camera<double>& calib);
	extern CGV_API void set_rgbd_calibration_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, const rgbd::rgbd_calibration& calib);
}
#include <cgv/config/lib_end.h>