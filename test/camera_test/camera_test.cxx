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
#include <point_cloud/point_cloud.h>
#include <rgbd_capture/frame.h>
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


struct rgbd_kinect_azure
{
	const cgv::math::camera<double>* c_ptr = 0;
	bool map_depth_to_point(int x, int y, int depth, float* point_ptr) const
	{
		// 
		const auto& c = *c_ptr;

		double fx_d = 1.0 / c.s(0);
		double fy_d = 1.0 / c.s(1);
		double cx_d = c.c(0);
		double cy_d = c.c(1);
		// set 0.001 for current vr_rgbd
		double d = 0.001 * depth * 1.000;
		d = 1;
		double x_d = (x - cx_d) * fx_d;
		double y_d = (y - cy_d) * fy_d;

		float uv[2], xy[2];
		uv[0] = float(x_d);
		uv[1] = float(y_d);
		bool valid = false;

		if (transformation_unproject_internal(uv, xy, valid)) {
			point_ptr[0] = float(xy[0] * d);
			point_ptr[1] = float(xy[1] * d);
			point_ptr[2] = float(d);
			return valid;
		}
		return false;
	}
	bool transformation_iterative_unproject(const float* uv, float* xy, bool& valid, unsigned int max_passes) const
	{
		valid = true;
		float Jinv[2 * 2];
		float best_xy[2] = { 0.f, 0.f };
		float best_err = FLT_MAX;

		for (unsigned int pass = 0; pass < max_passes; pass++) {
			float p[2];
			float J[2 * 2];
			if (!transformation_project_internal(xy, p, valid, J))
			{
				return false;
			}
			if (!valid)
			{
				return true;
			}

			float err_x = uv[0] - p[0];
			float err_y = uv[1] - p[1];
			float err = err_x * err_x + err_y * err_y;
			if (err >= best_err) {
				xy[0] = best_xy[0];
				xy[1] = best_xy[1];
				break;
			}

			best_err = err;
			best_xy[0] = xy[0];
			best_xy[1] = xy[1];
			invert_2x2(J, Jinv);
			if (pass + 1 == max_passes || best_err < 1e-22f) {
				break;
			}

			float dx = Jinv[0] * err_x + Jinv[1] * err_y;
			float dy = Jinv[2] * err_x + Jinv[3] * err_y;

			xy[0] += dx;
			xy[1] += dy;
		}
		if (best_err > 1e-6f)
		{
			valid = false;
		}
		return true;
	}
	bool transformation_project_internal(const float xy[2], float point2d[2], bool& valid, float J_xy[2 * 2]) const
	{
		const auto& c = *c_ptr;
		float max_radius_for_projection = float(c.max_radius_for_projection);
		valid = true;

		float xp = float(xy[0] - c.dc(0));
		float yp = float(xy[1] - c.dc(1));

		float xp2 = xp * xp;
		float yp2 = yp * yp;
		float xyp = xp * yp;
		float rs = xp2 + yp2;
		float rm = max_radius_for_projection * max_radius_for_projection;
		if (rs > rm) {
			valid = false;
			return true;
		}
		float rss = rs * rs;
		float rsc = rss * rs;
		float a = float(1.f + c.k[0] * rs + c.k[1] * rss + c.k[2] * rsc);
		float b = float(1.f + c.k[3] * rs + c.k[4] * rss + c.k[5] * rsc);
		float bi;
		if (b != 0.f) {
			bi = 1.f / b;
		}
		else {
			bi = 1.f;
		}
		float d = a * bi;

		float xp_d = xp * d;
		float yp_d = yp * d;

		float rs_2xp2 = rs + 2.f * xp2;
		float rs_2yp2 = rs + 2.f * yp2;

		xp_d += float(rs_2xp2 * c.p[1] + 2.f * xyp * c.p[0]);
		yp_d += float(rs_2yp2 * c.p[0] + 2.f * xyp * c.p[1]);

		float xp_d_cx = float(xp_d + c.dc(0));
		float yp_d_cy = float(yp_d + c.dc(1));

		point2d[0] = float(xp_d_cx * c.s(0) + c.c(0));
		point2d[1] = float(yp_d_cy * c.s(1) + c.c(1));

		if (J_xy == 0) {
			return true;
		}

		// compute Jacobian matrix
		float dudrs = float(c.k[0] + 2.f * c.k[1] * rs + 3.f * c.k[2] * rss);
		// compute d(b)/d(r^2)
		float dvdrs = float(c.k[3] + 2.f * c.k[4] * rs + 3.f * c.k[5] * rss);
		float bis = bi * bi;
		float dddrs = (dudrs * b - a * dvdrs) * bis;

		float dddrs_2 = dddrs * 2.f;
		float xp_dddrs_2 = xp * dddrs_2;
		float yp_xp_dddrs_2 = yp * xp_dddrs_2;
		// compute d(u)/d(xp)
		J_xy[0] = float(c.s(0) * (d + xp * xp_dddrs_2 + 6.f * xp * c.p[1] + 2.f * yp * c.p[0]));
		J_xy[1] = float(c.s(0) * (yp_xp_dddrs_2 + 2.f * yp * c.p[1] + 2.f * xp * c.p[0]));
		J_xy[2] = float(c.s(1) * (yp_xp_dddrs_2 + 2.f * xp * c.p[0] + 2.f * yp * c.p[1]));
		J_xy[3] = float(c.s(1) * (d + yp * yp * dddrs_2 + 6.f * yp * c.p[0] + 2.f * xp * c.p[1]));
		return true;
	}
	void invert_2x2(const float J[2 * 2], float Jinv[2 * 2]) const
	{
		float detJ = J[0] * J[3] - J[1] * J[2];
		float inv_detJ = 1.f / detJ;

		Jinv[0] = inv_detJ * J[3];
		Jinv[3] = inv_detJ * J[0];
		Jinv[1] = -inv_detJ * J[1];
		Jinv[2] = -inv_detJ * J[2];
	}
	bool transformation_unproject_internal(const float uv[2], float xy[2], bool valid) const
	{
		const auto& c = *c_ptr;
		double xp_d = uv[0] - c.dc(0);
		double yp_d = uv[1] - c.dc(0);

		double r2 = xp_d * xp_d + yp_d * yp_d;
		double r4 = r2 * r2;
		double r6 = r2 * r4;
		double r8 = r4 * r4;
		double a = 1 + c.k[0] * r2 + c.k[1] * r4 + c.k[2] * r6;
		double b = 1 + c.k[3] * r2 + c.k[4] * r4 + c.k[5] * r6;
		double ai;
		if (a != 0.f) {
			ai = 1.f / a;
		}
		else {
			ai = 1.f;
		}
		float di = float(ai * b);
		// solve the radial and tangential distortion
		double x_u = xp_d * di;
		double y_u = yp_d * di;

		// approximate correction for tangential params
		float two_xy = float(2.f * x_u * y_u);
		float xx = float(x_u * x_u);
		float yy = float(y_u * y_u);

		x_u -= (yy + 3.f * xx) * c.p[1] + two_xy * c.p[0];
		y_u -= (xx + 3.f * yy) * c.p[0] + two_xy * c.p[1];

		x_u += c.dc(0);
		y_u += c.dc(1);

		xy[0] = float(x_u);
		xy[1] = float(y_u);
		return transformation_iterative_unproject(uv, xy, valid, 20);
	}
};

class camera_test :
	public node,
	public drawable,
	public provider
{
protected:
	// rendering configuration
	cgv::math::camera<double> color_calib, depth_calib;
	rgbd_kinect_azure rka;
	point_render_style prs;

	enum class precision { float_precision, double_precision };
	precision prec = precision::double_precision;
	bool use_pc_shader_prog = true;
	bool use_azure_impl = false;
	float slow_down = 1.0f;
	unsigned sub_sample = 1;
	unsigned sub_line_sample = 4;
	unsigned nr_iterations = 20;
	bool skip_dark = true;
	float scale = 1.0f;
	float  xu0_rad = 2.1f;
	bool debug_xu0 = false;
	double xu_xd_lambda = 1.0f;
	float depth_lambda = 1.0f;
	double error_threshold = 0.0001;
	double error_scale = 1000;
	double epsilon = cgv::math::distortion_inversion_epsilon<double>();
	bool use_standard_epsilon = true;
	bool debug_colors = true;
	float random_offset = 0.001f;

	std::vector<vec3> P;
	std::vector<rgb8> C;

	rgbd::frame_type warped_color_frame;
	rgbd::frame_type depth_frame;
	point_cloud pc;

	bool read_frames()
	{
		/*
		if (!depth_frame.read("D:/data/images/kinect/door.dep"))
			return false;
		if (!warped_color_frame.read("D:/data/images/kinect/door.wrgb"))
			return false;
		if (!pc.read("D:/data/images/kinect/door.bpc"))
			return false;
		*/
		if (!depth_frame.read("D:/data/Anton/me_in_office.dep"))
			return false;
		if (!warped_color_frame.read("D:/data/Anton/me_in_office.wrgb"))
			return false;
		if (!pc.read("D:/data/Anton/me_in_office.bpc"))
			return false;
		construct_point_clouds();
		return true;
	}
	void construct_point_clouds()
	{
		P.clear();
		C.clear();
		switch (prec) {
		case precision::float_precision:
			construct_point_clouds_precision(cgv::math::camera<float>(depth_calib));
			break;
		default:
			construct_point_clouds_precision(depth_calib);
			break;
		}
	} 

	template <typename T>
	void construct_point_clouds_precision(const cgv::math::camera<T>& depth_calib)
	{
		T eps = use_standard_epsilon ? cgv::math::distortion_inversion_epsilon<T>() : T(epsilon);
		T sx = T(1) / depth_frame.width;
		T sy = T(1) / depth_frame.height;
		for (int y = 0; y < depth_frame.height; y += sub_sample) {
			for (int x = 0; x < depth_frame.width; x += sub_sample) {
				if (((x % sub_line_sample) != 0) && ((y % sub_line_sample) != 0))
					continue;
				uint8_t* pix_ptr = reinterpret_cast<uint8_t*>(&warped_color_frame.frame_data[(y * depth_frame.width + x) * warped_color_frame.get_nr_bytes_per_pixel()]);
				uint16_t depth = reinterpret_cast<const uint16_t&>(depth_frame.frame_data[(y * depth_frame.width + x) * depth_frame.get_nr_bytes_per_pixel()]);
				if (depth == 0)
					continue;
				//if (pix_ptr[0] + pix_ptr[1] + pix_ptr[2] < 16)
				//	continue;
				cgv::math::camera<T>::distortion_inversion_result dir;
				unsigned iterations = 1;
				cgv::math::fvec<T,2> xu, xd;
				if (debug_xu0) {
					xd = T(xu0_rad) * cgv::math::fvec<T, 2>(sx * x - T(0.5), sy * y - T(0.5));
					depth_calib.apply_distortion_model(xd, xu);
					std::default_random_engine g;
					std::uniform_real_distribution<T> d(-0.5, 0.5);
					xd += T(random_offset)* cgv::math::fvec<T, 2>(d(g), d(g));
					dir = depth_calib.invert_distortion_model(xu, xd, false, &iterations, eps, nr_iterations, slow_down);
				}
				else {
					xu = depth_calib.pixel_to_image_coordinates(cgv::math::fvec<T, 2>(T(x), T(y)));
					xd = xu;
					if (!use_azure_impl)
						dir = depth_calib.invert_distortion_model(xu, xd, true, &iterations, eps, nr_iterations, slow_down);
					else {
						vec3 p;
						if (rka.map_depth_to_point(x, y, 0, &p[0])) {
							xd[0] = p[0];
							xd[1] = p[1];
							dir = cgv::math::camera<T>::distortion_inversion_result::convergence;
							/*double err = sqrt((xd[0] - p[0]) * (xd[0] - p[0]) + (xd[1] - p[1]) * (xd[1] - p[1]));
							if (err > 0.1f) {
								rka.map_depth_to_point(x, y, 0, &p[0]);
								nr = depth_calib.invert_distortion_model(xu, xd, slow_down, nr_iterations);
							}*/
						}
						else
							dir = cgv::math::camera<T>::distortion_inversion_result::divergence;
					}
				}
				vec3 p(vec2(T(T(1) - xu_xd_lambda) * xu + T(xu_xd_lambda) * xd), 1.0f);
				p *= float((1 - depth_lambda) + depth_lambda * 0.001 * depth);
				P.push_back(p);

				cgv::math::fvec<T, 2> xu_rec;
				cgv::math::fmat<T, 2, 2> J;
				depth_calib.apply_distortion_model(xd, xu_rec, &J);
				T error = (xu_rec - xu).length();
				if (!debug_colors)
					C.push_back(rgb8(pix_ptr[2], pix_ptr[1], pix_ptr[0]));
				else {
					switch (dir) {
					case cgv::math::camera<T>::distortion_inversion_result::divergence:
						C.push_back(rgb8(255, 0, 255));
						break;
					case cgv::math::camera<T>::distortion_inversion_result::division_by_zero:
						C.push_back(rgb8(0, 255, 255));
						break;
					case cgv::math::camera<T>::distortion_inversion_result::max_iterations_reached:
						if (error > error_threshold)
							C.push_back(rgb8(cgv::media::color_scale(error_scale * error)));
						else {
							C.push_back(rgb8(pix_ptr[2], pix_ptr[1], pix_ptr[0]));
						}
						break;
					case cgv::math::camera<T>::distortion_inversion_result::out_of_bounds:
						C.push_back(rgb8(128, 128, 255));
						break;
					default:
						C.push_back(rgb8(pix_ptr[2], pix_ptr[1], pix_ptr[0]));
						break;
					}
				}
			}
		}
	}

	bool read_calibs()
	{
		nlohmann::json j;
		std::ifstream is("D:/data/Anton/me_in_office.json");
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
	cgv::render::shader_program pc_prog;
	cgv::render::view* view_ptr = 0;
public:
	camera_test()
	{
		rka.c_ptr = &depth_calib;
		set_name("camera_test");
		prs.point_size = 5;
		prs.blend_width_in_pixel = 0;
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &sub_sample || member_ptr == &nr_iterations || member_ptr == &slow_down ||
			member_ptr == & sub_line_sample ||
			member_ptr == & xu0_rad ||
			member_ptr == & prec ||
			member_ptr == &random_offset ||
			member_ptr == &use_standard_epsilon ||
			member_ptr == & epsilon ||
			member_ptr == & scale ||
			member_ptr == & use_azure_impl ||
			member_ptr == & debug_colors ||
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
		add_member_control(this, "use_pc_shader_prog", use_pc_shader_prog, "toggle");
		add_member_control(this, "sub_sample", sub_sample, "value_slider", "min=1;max=16;ticks=true");
		add_member_control(this, "sub_line_sample", sub_line_sample, "value_slider", "min=1;max=16;ticks=true");
		add_member_control(this, "use_azure_impl", use_azure_impl, "toggle");
		add_member_control(this, "slow_down", slow_down, "value_slider", "min=0;max=1;ticks=true");
		add_member_control(this, "precision", prec, "dropdown", "enums='float,double'");
		add_member_control(this, "nr_iterations", nr_iterations, "value_slider", "min=0;max=30;ticks=true");
		add_member_control(this, "use_standard_epsilon", use_standard_epsilon, "check");
		add_member_control(this, "epsilon", epsilon, "value_slider", "min=0.000000001;step=0.00000000001;max=0.001;ticks=true;log=true");
		add_member_control(this, "debug_colors", debug_colors, "toggle");
		add_member_control(this, "scale", scale, "value_slider", "min=0.5;max=10;ticks=true");
		add_member_control(this, "max_radius_for_projection", depth_calib.max_radius_for_projection, "value_slider", "min=0.5;max=10;ticks=true");
		add_member_control(this, "debug_xu0", debug_xu0, "toggle");
		add_member_control(this, "random_offset", random_offset, "value_slider", "min=0.00001;max=0.01;step=0.0000001;ticks=true;log=true");
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
		return pc_prog.build_program(ctx, "rgbd_pc.glpr", true);
	}
	void destruct(context& ctx)
	{
		ref_point_renderer(ctx, -1);
		pc_prog.destruct(ctx);
	}
	void init_frame(context& ctx)
	{
		if (!view_ptr)
			view_ptr = find_view_as_node();
		pc_aam.init(ctx);
	}
	void draw_points(context& ctx, size_t nr_elements, const vec3* P, const rgb8* C)
	{
		glDisable(GL_CULL_FACE);
		if (use_pc_shader_prog && pc_prog.is_linked()) {
			pc_prog.enable(ctx);
			pc_prog.disable(ctx);
		}
		else {
			auto& pr = ref_point_renderer(ctx);
			if (view_ptr)
				pr.set_y_view_angle(float(view_ptr->get_y_view_angle()));
			pr.set_render_style(prs);
			pr.enable_attribute_array_manager(ctx, pc_aam);
			pr.set_position_array(ctx, P, nr_elements);
			pr.set_color_array(ctx, C, nr_elements);
			pr.render(ctx, 0, nr_elements);
		}
		glEnable(GL_CULL_FACE);
	}
	void draw(context& ctx)
	{
		draw_points(ctx, P.size(), &P.front(), &C.front());
		//draw_points(ctx, pc.get_nr_points(), &pc.pnt(0), &pc.clr(0));
	}
};

object_registration<camera_test> camera_reg("");
