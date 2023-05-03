#include <cgv/base/node.h>
#include <cgv/math/camera.h>
#include <cgv/media/color_scale.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/register.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/point_renderer.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/a_buffer.h>
#include <random>
#include "../../3rd/json/nlohmann/json.hpp"
#include "../../3rd/json/cgv_json/math.h"
#include <fstream>
#include <cgv/utils/file.h>
#include <cgv/utils/convert.h>

using namespace cgv::base;
using namespace cgv::data;
using namespace cgv::signal;
using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::render::gl;

/// frame size in pixels
struct frame_size
{
	/// width of frame in pixel
	int width;
	/// height of frame in pixel 
	int height;
};
/// format of individual pixels
enum PixelFormat {
	PF_I, // infrared

	/* TODO: add color formats in other color spaces like YUV */

	PF_RGB,   // 24 or 32 bit rgb format with byte alignment
	PF_BGR,   // 24 or 24 bit bgr format with byte alignment
	PF_RGBA,  // 32 bit rgba format
	PF_BGRA,  // 32 bit brga format
	PF_BAYER, // 8 bit per pixel, raw bayer pattern values

	PF_DEPTH,
	PF_DEPTH_AND_PLAYER,
	PF_POINTS_AND_TRIANGLES,
	PF_CONFIDENCE
};
/// format of a frame
struct frame_format : public frame_size
{
	/// format of pixels
	PixelFormat pixel_format;
	// total number of bits per pixel
	unsigned nr_bits_per_pixel;
	/// return number of bytes per pixel (ceil(nr_bits_per_pixel/8))
	unsigned get_nr_bytes_per_pixel() const;
	/// buffer size; returns width*height*get_nr_bytes_per_pixel()
	unsigned buffer_size;
	/// standard computation of the buffer size member
	void compute_buffer_size();
};
/// struct to store single frame
struct frame_info : public frame_format
{
	///
	unsigned frame_index;
	/// 
	double time;
};
/// struct to store single frame
struct frame_type : public frame_info
{
	/// vector with RAW frame data 
	std::vector<uint8_t> frame_data;
	/// check whether frame data is allocated
	bool is_allocated() const;
	/// write to file
	bool write(const std::string& fn) const;
	/// read from file
	bool read(const std::string& fn);
};
std::string get_frame_extension(const frame_format& ff)
{
	static const char* exts[] = {
		"ir", "rgb", "bgr", "rgba", "bgra", "byr", "dep", "d_p", "p_tri"
	};
	return std::string(exts[ff.pixel_format]) + to_string(ff.nr_bits_per_pixel);
}
std::string compose_file_name(const std::string& file_name, const frame_format& ff, unsigned idx)
{
	std::string fn = file_name;

	std::stringstream ss;
	ss << std::setfill('0') << std::setw(10) << idx;

	fn += ss.str();
	return fn + '.' + get_frame_extension(ff);
}
/// return number of bytes per pixel (ceil(nr_bits_per_pixel/8))
unsigned frame_format::get_nr_bytes_per_pixel() const
{
	return nr_bits_per_pixel / 8 + ((nr_bits_per_pixel & 7) == 0 ? 0 : 1);
}
/// standard computation of the buffer size member
void frame_format::compute_buffer_size()
{
	buffer_size = width * height * get_nr_bytes_per_pixel();
}
/// check whether frame data is allocated
bool frame_type::is_allocated() const
{
	return !frame_data.empty();
}
/// write to file
bool frame_type::write(const std::string& fn) const
{
	assert(buffer_size == frame_data.size());
	return
		cgv::utils::file::write(fn, reinterpret_cast<const char*>(this), sizeof(frame_format), false) &&
		cgv::utils::file::append(fn, reinterpret_cast<const char*>(&frame_data.front()), frame_data.size(), false);
}
/// read from file
bool frame_type::read(const std::string& fn)
{
	if (!cgv::utils::file::read(fn,
		reinterpret_cast<char*>(static_cast<frame_format*>(this)),
		sizeof(frame_format), false))
		return false;
	frame_data.resize(buffer_size);
	return
		cgv::utils::file::read(fn,
			reinterpret_cast<char*>(&frame_data.front()), buffer_size, false,
			sizeof(frame_format));
}

class camera_test :
	public node,
	public drawable,
	public provider
{
protected:
	// rendering configuration
	cgv::math::camera<double> color_calib, depth_calib;
	
	point_render_style prs;

	float beta = 0.5f;
	unsigned sub_sample = 4;
	unsigned sub_line_sample = 8;
	unsigned nr_iterations = 20;
	bool skip_dark = true;
	float scale = 1.0f;
	float  xu0_rad = 2.1f;
	bool debug_xu0 = false;
	double xu_xd_lambda = 1.0f;
	float depth_lambda = 0.0f;
	double error_threshold = 0.0001;
	double error_scale = 1000;
	
	std::vector<vec3> P;
	std::vector<rgb8> C;

	frame_type warped_color_frame;
	frame_type depth_frame;

	bool read_frames()
	{
		if (!depth_frame.read("D:/data/images/kinect/door.dep"))
			return false;
		if (!warped_color_frame.read("D:/data/images/kinect/door.wrgb"))
			return false;

		construct_point_clouds();
		return true;
	}
	void construct_point_clouds()
	{
		P.clear(); 
		C.clear();

		float sx = 1.0f / depth_frame.width;
		float sy = 1.0f / depth_frame.height;
		for (int y = 0; y < depth_frame.height; y += sub_sample) {
			for (int x = 0; x < depth_frame.width; x += sub_sample) {
				if (((x % sub_line_sample) != 0) && ((y % sub_line_sample) != 0))
					continue;
				uint8_t* pix_ptr = &warped_color_frame.frame_data[(y * depth_frame.width + x) * warped_color_frame.get_nr_bytes_per_pixel()];
				uint16_t depth = reinterpret_cast<const uint16_t&>(depth_frame.frame_data[(y * depth_frame.width + x) * depth_frame.get_nr_bytes_per_pixel()]);
				if (pix_ptr[0] + pix_ptr[1] + pix_ptr[2] < 16)
					continue;
				unsigned nr = 1;
				dvec2 xu, xd;
				if (debug_xu0) {
					xd = xu0_rad * vec2(sx * x - 0.5f, sy * y - 0.5f);
					depth_calib.apply_distortion_model(xd, xu);
				}
				else {
					xu = depth_calib.pixel_to_image_coordinates(dvec2(x, y));
					xd = xu;
					nr = depth_calib.invert_distortion_model(xu, xd, beta, nr_iterations);
				}
				bool success = nr > 0 || nr_iterations == 0;

				vec3 p(vec2((1.0 - xu_xd_lambda) * xu + xu_xd_lambda * xd), 1.0);
				p *= float((1 - depth_lambda) + depth_lambda * 0.001 * depth);
				P.push_back(p);

				dvec2 xu_rec;
				depth_calib.apply_distortion_model(xd, xu_rec);
				double error = (xu_rec - xu).length();
				if (!success)
					C.push_back(rgb8(255, 0, 255));
				else {
					if (nr > nr_iterations) {
						if (nr > 2 * nr_iterations)
							C.push_back(rgb8(cgv::media::color_scale(scale * (nr - 2.0 * nr_iterations) / nr_iterations, cgv::media::CS_BLUE)));
						else
							C.push_back(rgb8(cgv::media::color_scale(scale * (nr - 1.0 * nr_iterations) / nr_iterations, cgv::media::CS_GREEN)));
					}
					else {
						if (error > error_threshold)
							C.push_back(rgb8(cgv::media::color_scale(error_scale * error)));
						else
							C.push_back(rgb8(pix_ptr[0], pix_ptr[1], pix_ptr[2]));
					}
				}
			}
		}
	}

	bool read_calibs()
	{
		nlohmann::json j;
		std::ifstream is("D:/data/images/kinect/kinect.json");
		if (is.fail())
			return false;
		is >> j;
		std::string serial;
		j.at("serial").get_to(serial);
		j.at("color_calib").get_to(color_calib);
		j.at("depth_calib").get_to(depth_calib);
		depth_calib.max_radius_for_projection = 1.7f;
		color_calib.max_radius_for_projection = 1.7f;
		return true;
	}
	attribute_array_manager pc_aam;
	cgv::render::view* view_ptr = 0;
public:
	camera_test()
	{
		set_name("camera_test");
		prs.point_size = 5;
		prs.blend_width_in_pixel = 0;
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &sub_sample || member_ptr == &nr_iterations || member_ptr == &beta ||
			member_ptr == & sub_line_sample ||
			member_ptr == & xu0_rad ||
			member_ptr == & scale ||
			member_ptr == & xu_xd_lambda ||
			member_ptr == &depth_calib.max_radius_for_projection ||
			member_ptr == &debug_xu0 ||
			member_ptr == &depth_lambda ||
			member_ptr == & error_threshold ||
			member_ptr == & error_scale) {
			construct_point_clouds();
		}
		update_member(member_ptr);
		post_redraw();
	}
	std::string get_type_name() const { return "camera_test"; }

	void create_gui()
	{
		add_decorator("a_buffer", "heading", "level=1");
		add_member_control(this, "sub_sample", sub_sample, "value_slider", "min=1;max=16;ticks=true");
		add_member_control(this, "sub_line_sample", sub_line_sample, "value_slider", "min=1;max=16;ticks=true");
		add_member_control(this, "beta", beta, "value_slider", "min=0;max=1;ticks=true");
		add_member_control(this, "nr_iterations", nr_iterations, "value_slider", "min=0;max=30;ticks=true");
		add_member_control(this, "scale", scale, "value_slider", "min=0.5;max=10;ticks=true");
		add_member_control(this, "max_radius_for_projection", depth_calib.max_radius_for_projection, "value_slider", "min=0.5;max=10;ticks=true");
		add_member_control(this, "debug_xu0", debug_xu0, "toggle");
		add_member_control(this, "xu0_rad", xu0_rad, "value_slider", "min=0.5;max=5;ticks=true");
		add_member_control(this, "xu_xd_lambda", xu_xd_lambda, "value_slider", "min=0;max=1;ticks=true");
		add_member_control(this, "depth_lambda", depth_lambda, "value_slider", "min=0;max=1;ticks=true");
		add_member_control(this, "error_threshold", error_threshold, "value_slider", "min=0.000001;max=0.1;step=0.0000001;log=true;ticks=true");
		add_member_control(this, "error_scale", error_scale, "value_slider", "min=1;max=10000;log=true;ticks=true");

		if (begin_tree_node("point style", prs)) {
			align("\a");
			add_gui("point style", prs);
			align("\a");
			end_tree_node(prs);
		}

	}
	bool init(context& ctx)
	{
		ctx.set_bg_clr_idx(4);
		ref_point_renderer(ctx, 1);
		read_calibs();
		read_frames();
		return true;
	}
	void destruct(context& ctx)
	{
		ref_point_renderer(ctx, -1);
	}
	void init_frame(context& ctx)
	{
		if (!view_ptr)
			view_ptr = find_view_as_node();
		pc_aam.init(ctx);
	}
	void draw_points(context& ctx, const std::vector<vec3>& P, const std::vector<rgb8>& C)
	{
		auto& pr = ref_point_renderer(ctx);
		if (view_ptr)
			pr.set_y_view_angle(float(view_ptr->get_y_view_angle()));
		pr.set_render_style(prs);
		pr.enable_attribute_array_manager(ctx, pc_aam);
		pr.set_position_array(ctx, P);
		pr.set_color_array(ctx, C);
		glDisable(GL_CULL_FACE);
		pr.render(ctx, 0, P.size());
		glEnable(GL_CULL_FACE);
	}
	void draw(context& ctx)
	{
		draw_points(ctx, P, C);
	}
};

object_registration<camera_test> camera_reg("");
