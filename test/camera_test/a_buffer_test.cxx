#include <cgv/base/node.h>
#include <cgv/math/camera.h>
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

using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::render::gl;

class camera_test :
	public node,
	public drawable,
	public provider
{
protected:
	// rendering configuration
	cgv::math::camera<double> color_calib, depth_calib;
	
	point_render_style prs;

	std::vector<vec3> P;
	std::vector<rgb> C;

	bool read_calibs()
	{
		nlohmann::json j;
		std::ifstream is("D:/kinect.json");
		if (is.fail())
			return false;
		is >> j;
		std::string serial;
		j.at("serial").get_to(serial);
		j.at("color_calib").get_to(color_calib);
		j.at("depth_calib").get_to(depth_calib);
		return true;
	}
	attribute_array_manager pc_aam;
	cgv::render::view* view_ptr = 0;
public:
	camera_test()
	{
		set_name("camera_test");
	}
	void on_set(void* member_ptr)
	{
		update_member(member_ptr);
		post_redraw();
	}
	std::string get_type_name() const { return "camera_test"; }

	void create_gui()
	{
		add_decorator("a_buffer", "heading", "level=1");
		if (begin_tree_node("point style", prs)) {
			align("\a");
			add_gui("point style", prs);
			align("\a");
			end_tree_node(prs);
		}

	}
	bool init(context& ctx)
	{
		ref_point_renderer(ctx, 1);
		read_calibs();
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
	void draw_points(context& ctx)
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
		draw_points(ctx);
	}
};

object_registration<camera_test> camera_reg("");
