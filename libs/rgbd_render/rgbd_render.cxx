#include "rgbd_render.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace rgbd {

bool create_or_update_texture_from_frame(cgv::render::context& ctx,
	cgv::render::texture& tex, const rgbd::frame_type& frame)
{
	const cgv::data::ComponentFormat cfs[] = {
		cgv::data::CF_R,     // PF_I
		cgv::data::CF_RGB,   // PF_RGB
		cgv::data::CF_BGR,   // PF_BGR
		cgv::data::CF_RGBA,  // PF_RGBA
		cgv::data::CF_BGRA,  // PF_BGRA
		cgv::data::CF_UNDEF, // PF_BAYER
		cgv::data::CF_R,     // PF_DEPTH
		cgv::data::CF_UNDEF, // PF_DEPTH_AND_PLAYER
		cgv::data::CF_UNDEF, // PF_POINTS_AND_TRIANGLES
		cgv::data::CF_R      // PF_CONFIDENCE
};
	auto cf = cfs[frame.pixel_format];
	if (cf == cgv::data::CF_UNDEF)
		return false;
	cgv::type::info::TypeId type = cgv::type::info::TI_UINT8;
	switch (frame.pixel_format) {
	case rgbd::PF_RGB:
		if (frame.get_nr_bytes_per_pixel() == 4)
			cf = cgv::data::CF_RGBA;
		break;
	case rgbd::PF_BGR:
		if (frame.get_nr_bytes_per_pixel() == 4)
			cf = cgv::data::CF_BGRA;
		break;
	case rgbd::PF_I:
	case rgbd::PF_DEPTH:
	case rgbd::PF_CONFIDENCE:
		switch (frame.get_nr_bytes_per_pixel()) {
		case 1:break;
		case 2:type = cgv::type::info::TI_UINT16; break;
		default:type = cgv::type::info::TI_UINT32; break;
		}
		break;
	}
	cgv::data::data_format df(frame.width, frame.height, type, cf);
	cgv::data::const_data_view dv(&df, frame.frame_data.data());
	if (tex.is_created()) {
		if (tex.get_nr_dimensions() == 2 &&
			tex.get_width() == frame.width && tex.get_height()) {
			tex.replace(ctx, 0, 0, dv);
			return true;
		}
		tex.destruct(ctx);
	}
	tex.create(ctx, dv);
	return true;
}
void construct_rgbd_render_data(
	const rgbd::frame_type& depth_frame,
	std::vector<cgv::usvec3>& sP,
	uint16_t sub_sample, uint16_t sub_line_sample)
{
	sP.clear();
	for (uint16_t y = 0; y < depth_frame.height; y += sub_sample)
		for (uint16_t x = 0; x < depth_frame.width; x += sub_sample) {
			if (((x % sub_line_sample) != 0) && ((y % sub_line_sample) != 0))
				continue;
			uint16_t depth = reinterpret_cast<const uint16_t&>(depth_frame.frame_data[(y * depth_frame.width + x) * depth_frame.get_nr_bytes_per_pixel()]);
			if (depth == 0)
				continue;
			sP.push_back(cgv::usvec3(x, y, depth));
		}
}
void construct_rgbd_render_data_with_color(
	const rgbd::frame_type& depth_frame,
	const rgbd::frame_type& warped_color_frame,
	std::vector<cgv::usvec3>& sP,
	std::vector<cgv::rgb8>& sC,
	uint16_t sub_sample, uint16_t sub_line_sample)
{
	sP.clear();
	sC.clear();
	for (uint16_t y = 0; y < depth_frame.height; y += sub_sample)
		for (uint16_t x = 0; x < depth_frame.width; x += sub_sample) {
			if (((x % sub_line_sample) != 0) && ((y % sub_line_sample) != 0))
				continue;
			const uint8_t* pix_ptr = reinterpret_cast<const uint8_t*>(&warped_color_frame.frame_data[(y * depth_frame.width + x) * warped_color_frame.get_nr_bytes_per_pixel()]);
			uint16_t depth = reinterpret_cast<const uint16_t&>(depth_frame.frame_data[(y * depth_frame.width + x) * depth_frame.get_nr_bytes_per_pixel()]);
			if (depth == 0)
				continue;
			sP.push_back(cgv::usvec3(x, y, depth));
			sC.push_back(cgv::rgb8(pix_ptr[2], pix_ptr[1], pix_ptr[0]));
		}
}

void set_camera_calibration_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, const std::string& name, const cgv::math::camera<double>& calib)
{
	prog.set_uniform(ctx, name + ".w", int(calib.w));
	prog.set_uniform(ctx, name + ".h", int(calib.h));
	prog.set_uniform(ctx, name + ".max_radius_for_projection", float(calib.max_radius_for_projection));
	prog.set_uniform(ctx, name + ".dc", cgv::vec2(calib.dc));
	prog.set_uniform(ctx, name + ".k[0]", float(calib.k[0]));
	prog.set_uniform(ctx, name + ".k[1]", float(calib.k[1]));
	prog.set_uniform(ctx, name + ".k[2]", float(calib.k[2]));
	prog.set_uniform(ctx, name + ".k[3]", float(calib.k[3]));
	prog.set_uniform(ctx, name + ".k[4]", float(calib.k[4]));
	prog.set_uniform(ctx, name + ".k[5]", float(calib.k[5]));
	prog.set_uniform(ctx, name + ".p[0]", float(calib.p[0]));
	prog.set_uniform(ctx, name + ".p[1]", float(calib.p[1]));
	prog.set_uniform(ctx, name + ".skew", float(calib.skew));
	prog.set_uniform(ctx, name + ".c", cgv::vec2(calib.c));
	prog.set_uniform(ctx, name + ".s", cgv::vec2(calib.s));
}
void set_rgbd_calibration_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, const rgbd::rgbd_calibration& calib)
{
	prog.set_uniform(ctx, "depth_scale", float(calib.depth_scale));
	set_camera_calibration_uniforms(ctx, prog, "depth_calib", calib.depth);
	set_camera_calibration_uniforms(ctx, prog, "color_calib", calib.color);
	prog.set_uniform(ctx, "color_rotation", cgv::mat3(cgv::math::pose_orientation(calib.color.pose)));
	prog.set_uniform(ctx, "color_translation", cgv::vec3(cgv::math::pose_position(calib.color.pose)));
}

}

#ifdef REGISTER_SHADER_FILES
#include <cgv/base/register.h>
#include <rgbd_render_shader_inc.h>
#endif
