#include <cgv/base/node.h>
#include <cgv/signal/rebind.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/application.h>
#include <cgv/render/drawable.h>
#include <cgv/render/view.h>
#include <cgv/math/vec.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>
#include <cgv_gl/sphere_renderer.h>
#include <vector>

using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::signal;
using namespace cgv::math;

class viewport_sample :
	public cgv::base::node,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::gui::event_handler,  /// derive from handler to receive events and to be asked for a help string
	public cgv::gui::provider,
	public cgv::render::drawable     /// derive from drawable for drawing the cube
{
private:
	int drag_pnt_idx = -1;
	bool is_drag_action = false;
	cgv::dmat4 MPW;
public:
	///
	int col_idx = -1, row_idx = -1;
	bool mouse_over_panel = false;
	int n = 3;
	int m = 2;
	int offset_x = 0, offset_y = 0;
	std::vector<cgv::vec3> points;
	std::vector<cgv::rgb> colors;
	cgv::render::sphere_render_style srs;
	cgv::render::TextAlignment align = TA_TOP;
	cgv::render::view* view_ptr = 0;

	viewport_sample() : node("viewport sample") 
	{
		srs.radius = 0.02f;
		srs.map_color_to_material = cgv::render::CM_COLOR;
		srs.surface_color = cgv::rgb(1, 0, 0);

	}
	/// return name of type
	std::string get_type_name() const 
	{ 
		return "viewport_sample"; 
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &n || member_ptr == &m) {
			if (view_ptr)
				view_ptr->enable_viewport_splitting(n, m);
		}
		post_redraw();
	}
	void stream_help(std::ostream& os)
	{
//		os << "toggle sampling type (S)" << std::endl;
	}
	bool init(context& ctx)
	{
		view_ptr = find_view_as_node();
		view_ptr->enable_viewport_splitting(n, m);
		for (int i = 0; i < (int)n; ++i)
			for (int j = 0; j < (int)m; ++j) {
				view_ptr->enable_viewport_individual_view(i, j);
				auto& view = view_ptr->ref_viewport_view(i, j);
				view.set_focus(0, 0, 0);
				cgv::vec3 view_dir((float)i - 1, 0, 2.0f * (j - 0.5f));
				view.set_view_dir(view_dir);
				view.set_view_up_dir(0, 1, 0);
			}
		ref_sphere_renderer(ctx, 1);
		return true;
	}
	/// init renderer
	void clear(cgv::render::context& ctx)
	{
		view_ptr->disable_viewport_splitting();
		ref_sphere_renderer(ctx, -1);
	}
	void draw(context& ctx)
	{
		//std::cout << "------------" << std::endl;
		ctx.enable_font_face(cgv::media::font::default_font()->get_font_face(cgv::media::font::FFA_BOLD), 30);
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < m; ++j) {
				view_ptr->activate_split_viewport(ctx, i, j);
				if (mouse_over_panel && col_idx == i && row_idx == j)
					ctx.set_color(cgv::rgb(1, 0.6f, 0.6f));
				else
					ctx.set_color(cgv::rgb(0.6f, 0.6f, 0.6f));
				glDepthMask(GL_FALSE);
				{
					auto& prog = ctx.ref_default_shader_program();
					prog.enable(ctx);
					cgv::render::gl::cover_screen(ctx, &prog);
					prog.disable(ctx);
				}
				glDepthMask(GL_TRUE);
				{
					auto& prog = ctx.ref_surface_shader_program();
					prog.set_uniform(ctx, "map_color_to_material", 3);
					prog.enable(ctx);
					ctx.set_color(cgv::rgb(0.7f, 0.5f, 0.4f));
					ctx.tesselate_unit_cube();
					prog.disable(ctx);
				}
				if (!points.empty()) {
					auto& sr = ref_sphere_renderer(ctx);
					sr.set_render_style(srs);
					sr.set_position_array(ctx, points);
					sr.set_color_array(ctx, colors);
					sr.set_y_view_angle(float(view_ptr->get_y_view_angle()));
					sr.render(ctx, 0, points.size());

					ctx.set_color(cgv::rgb(0, 0, 0));
					for (size_t k = 0; k < points.size(); ++k) {
						std::string text = cgv::utils::to_string(k);
						int x, y, vp[4];
						glGetIntegerv(GL_VIEWPORT, vp);
						ctx.put_cursor_coords(points[k].to_vec(), x, y);
						ctx.set_cursor(points[k].to_vec(), text, align, offset_x, offset_y);
						//ctx.set_cursor(x, y);
						ctx.output_stream() << text << std::endl;
						//if (mouse_over_panel && col_idx == i && row_idx == j)
						//	std::cout << "*";
						//std::cout << i << "," << j << "(" << vp[0] << "," << vp[1] << ":" << vp[2] << "x" << vp[3] << ") p[" << k << "]=" << x << "," << y << std::endl;
					}
				}
				view_ptr->deactivate_split_viewport(ctx);
			}
		}
	}
	/// check if a world point is close enough to the drawing square
	bool is_inside(const cgv::vec3& p) const
	{
		float bound = 1.0f + 1.2f * srs.radius * srs.radius_scale;
		return fabs(p(2)) <= bound && fabs(p(0)) <= bound && fabs(p(1)) <= bound;
	}
	cgv::vec3 project(const cgv::vec3& p3d) const
	{
		cgv::vec3 p = p3d;
		for (unsigned i = 0; i < 3; ++i) {
			if (p(i) < -1)
				p(i) = -1;
			if (p(i) > 1)
				p(i) = 1;
		}
		return p;
	}
	int find_closest(const cgv::vec3& p) const
	{
		if (points.empty())
			return -1;
		double min_dist = length(points[0] - p);
		int res = 0;
		for (int i = 1; i < (int)points.size(); ++i) {
			if (length(points[i] - p) < min_dist) {
				min_dist = length(points[i] - p);
				res = i;
			}
		}
		return res;
	}
	void create_gui()
	{
		add_member_control(this, "n", n, "value_slider", "min=1;max=10");
		add_member_control(this, "m", m, "value_slider", "min=1;max=10");
		add_member_control(this, "offset_x", offset_x, "value_slider", "min=-10;max=10;ticks=true");
		add_member_control(this, "offset_y", offset_y, "value_slider", "min=-10;max=10;ticks=true");
		add_member_control(this, "align", (cgv::type::DummyEnum&)align, "dropdown", "enums='center,left,right,top=4,topleft,topright,bottom=8,bottomleft,bottomright'");
	}
	bool handle(event& e)
	{
		if (e.get_kind() == EID_MOUSE) {
			mouse_event& me = static_cast<mouse_event&>(e);
			const cgv::dmat4* MPW_ptr = 0;
			int x_gl = me.get_x();
			int y_gl = get_context()->get_height() - 1 - me.get_y();
			if (me.get_action() == MA_LEAVE) {
				if (mouse_over_panel) {
					mouse_over_panel = false;
					post_redraw();
				}
			}
			else {
				view_ptr->get_modelview_projection_window_matrices(x_gl, y_gl,
					get_context()->get_width(), get_context()->get_height(),
					&MPW_ptr, 0, 0, 0, &col_idx, &row_idx);
				mouse_over_panel = true;
				//std::cout << "col_idx=" << col_idx << ", row_idx=" << row_idx << std::endl;
				post_redraw();
			}
			switch (me.get_action()) {
			case MA_PRESS:
				if (me.get_button() == MB_LEFT_BUTTON && me.get_modifiers() == EM_CTRL) {
					cgv::vec3 p = get_context()->get_model_point(x_gl, y_gl, *MPW_ptr);
					if (is_inside(p)) {
						p = project(p);
						drag_pnt_idx = -1;
						int i = find_closest(p);
						if (i != -1) {
							if (length(points[i] - p) < 1.2f * srs.radius * srs.radius_scale)
								drag_pnt_idx = i;
						}
						is_drag_action = drag_pnt_idx == -1;
						if (is_drag_action) {
							points.push_back(p);
							auto c = 0.5f * (p + 1.0f);
							colors.push_back(cgv::rgb(c.x(), c.y(), c.z()));
							drag_pnt_idx = (int)points.size() - 1;
						}
						post_redraw();
						return true;
					}
				}
				break;
			case MA_RELEASE:
				if (drag_pnt_idx != -1) {
					if (!is_drag_action) {
						points.erase(points.begin() + drag_pnt_idx);
						colors.erase(colors.begin() + drag_pnt_idx);
					}
					drag_pnt_idx = -1;
					post_redraw();
					return true;
				}
				break;
			case MA_DRAG:
				if (drag_pnt_idx != -1) {
					cgv::vec3 p = get_context()->get_model_point(x_gl, y_gl, *MPW_ptr);
					if (!is_inside(p)) {
						points.erase(points.begin() + drag_pnt_idx);
						colors.erase(colors.begin() + drag_pnt_idx);
						drag_pnt_idx = -1;
					}
					else {
						is_drag_action = true;
						points[drag_pnt_idx] = project(p);
					}
					post_redraw();
					return true;
				}
				break;
			}
		}
		return false;
	}
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const 
	{
		return "example/viewport_sample"; 
	}
};

#include <cgv/base/register.h>

factory_registration<viewport_sample> vp_fac("New/Events/Viewport Sample", 'V', true);
//object_registration<viewport_sample> vp_fac("Viewport Sample");

