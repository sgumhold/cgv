#include <cgv/gui/key_event.h>
#include <cgv/gui/key_control.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/provider.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/render/drawable.h>
#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/base/register.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv/math/ftransform.h>
#include <cgv/math/geom.h>

using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::gui;
using namespace cgv::math;
using namespace cgv::render;
using namespace cgv::utils;

class proj_geom_demo : public node, public drawable, public provider
{
protected:
	double tube_radius;
	cgv::dvec3   x[2];
	cgv::dvec3   l[2];
	bool show_plane, show_line_plane[2], show_point[2], show_line[2], show_axes;
	cgv::media::illum::surface_material axes_mat, surface_mat;
	sphere_render_style srs;
public:
	proj_geom_demo() :
		axes_mat(
			cgv::media::illum::BrdfType(cgv::media::illum::BT_OREN_NAYAR + cgv::media::illum::BT_COOK_TORRANCE),
		cgv::rgb(0.1f, 0.1f, 0.1f),
			0.1f, 0.5f, 0.2f
		),
		surface_mat(
			cgv::media::illum::BT_OREN_NAYAR,
			cgv::rgb(0.64f, 0.64f, 0.46f),
			1.0f, 0.0f, 0.2f
		)
	{
		surface_mat.set_transparency(0.3f);
		set_name("proj_geom_demo");
		x[0] = cgv::dvec3(1.0, 0.0, 1.0);
		x[1] = cgv::dvec3(0.0, 1.0, 1.0);
		l[0] = cross(x[0], x[1]);
		l[1] = cgv::dvec3(-0.5, 0.5, 0.2);
		tube_radius = 0.05;
		srs.radius = 0.15f;
		show_axes = true;
		srs.radius_scale = 1;
		show_plane = true;
		show_line_plane[0] = show_line_plane[1] = false;
		show_line[0] = show_line[1] = false;
		show_point[0] = show_point[1] = false;
	}
	void on_set(void* member_ptr)
	{
		update_member(&member_ptr);
		post_redraw();
		return;
	}
	bool init(context& ctx)
	{
		ref_sphere_renderer(ctx, 1);
		return true;
	}
	void destruct(context& ctx)
	{
		ref_sphere_renderer(ctx, -1);
	}
	void compute_line()
	{
		l[0] = cross(x[0], x[1]);
		for (unsigned c = 0; c < 3; ++c)
			update_member(&l[0][c]);
		post_redraw();
	}
	void compute_point()
	{
		x[0] = cross(l[0], l[1]);
		for (unsigned c = 0; c < 3; ++c)
			update_member(&x[0][c]);
		post_redraw();
	}
	void create_gui()
	{
		add_decorator("projective geometry demo", "heading", "level=2");
		connect_copy(add_button("compute line")->click, rebind(this, &proj_geom_demo::compute_line));
		connect_copy(add_button("compute point")->click, rebind(this, &proj_geom_demo::compute_point));
		if (begin_tree_node("visibility", show_plane)) {
			align("\a");
			add_member_control(this, "show_plane", show_plane, "toggle");
			add_member_control(this, "show_axes", show_axes, "toggle");
			add_member_control(this, "show_point_1", show_point[0], "toggle");
			add_member_control(this, "show_point_2", show_point[1], "toggle");
			add_member_control(this, "line_1", show_line[0], "toggle");
			add_member_control(this, "line_2", show_line[1], "toggle");
			add_member_control(this, "line_plane_1", show_line_plane[0], "toggle");
			add_member_control(this, "line_plane_2", show_line_plane[1], "toggle");
			align("\b");
			end_tree_node(show_plane);
		}

		if (begin_tree_node("dimensions", tube_radius)) {
			align("\a");
			add_member_control(this, "tube_radius", tube_radius, "value_slider", "min=0.02;max=5;ticks=true;log=true");
			add_member_control(this, "sphere_radius", srs.radius, "value_slider", "min=0.02;max=5;ticks=true;log=true");
			add_member_control(this, "scale", srs.radius_scale, "value_slider", "min=0.02;max=5;ticks=true;log=true");
			align("\b");
			end_tree_node(tube_radius);
		}
		if (begin_tree_node("points", x[0])) {
			align("\a");
			add_gui("x1", x[0], "vector", "main_label='';long_label=true;gui_type='value_slider';options='min=-1;max=3;ticks=true'");
			add_gui("x2", x[1], "vector", "main_label='';long_label=true;gui_type='value_slider';options='min=-1;max=3;ticks=true'");
			align("\b");
			end_tree_node(x[0]);
		}
		if (begin_tree_node("lines", l[0])) {
			align("\a");
			add_gui("l1", l[0], "vector", "main_label='';long_label=true;gui_type='value_slider';options='min=-1;max=1;ticks=true'");
			add_gui("l2", l[1], "vector", "main_label='';long_label=true;gui_type='value_slider';options='min=-1;max=1;ticks=true'");
			align("\b");
			end_tree_node(l[0]);
		}
		if (begin_tree_node("materials", axes_mat)) {
			align("\a");
			add_decorator("vectors", "heading", "level=3");
			add_gui("vector material", axes_mat);
			add_decorator("surface", "heading", "level=3");
			add_gui("surface material", surface_mat);
			align("\b");
			end_tree_node(axes_mat);
		}
		if (begin_tree_node("sphere style", srs)) {
			align("\a");
			add_gui("sphere render style", srs);
			align("\b");
			end_tree_node(srs);
		}
	}
	void arrow(context& c, const cgv::dvec3& b, const cgv::dvec3& e, const cgv::rgb& color)
	{
		c.set_color(color);
		double l = (b - e).length();
		c.tesselate_arrow(b, e, srs.radius_scale*tube_radius / l);
	}
	void line(context& ctx, const cgv::dvec3& l, const cgv::rgb& color)
	{
		ctx.set_color(color);
		double a = l[0], b = l[1], c = l[2], r = 5;
		double aa = a * a, bb = b * b, aabb = aa + bb, rr = r * r, cc = c * c;
		double d = rr * aabb - cc;
		if (d < 0)
			return;
		double dx = sqrt(bb * d);
		double dy = sqrt(aa * d);
		double y0 = (dy - b * c) / aabb;
		double y1 = -(dy + b * c) / aabb;
		double x0 = sqrt(rr - y0 * y0);
		if (fabs(a*x0 + b * y0 + c) > fabs(-a * x0 + b * y0 + c))
			x0 = -x0;
		double x1 = sqrt(rr - y1 * y1);
		if (fabs(a*x1 + b * y1 + c) > fabs(-a * x1 + b * y1 + c))
			x1 = -x1;
		//	(dx - a * c) / aabb;
		//double x0 = -(dx + a * c) / aabb;
		arrow(ctx, cgv::dvec3(x0, y0, 1), cgv::dvec3(x1, y1, 1), color);
	}
	void draw(context& c)
	{
		cgv::render::shader_program& prog = c.ref_surface_shader_program();
		prog.enable(c);
		prog.set_uniform(c, "map_color_to_material", 3);
		prog.set_uniform(c, "illumination_mode", 1);
		c.set_material(axes_mat);
		if (show_axes) {
			arrow(c, cgv::dvec3(0, 0, 0), cgv::dvec3(3, 0, 0), cgv::rgb(1, 0, 0));
			arrow(c, cgv::dvec3(0, 0, 0), cgv::dvec3(0, 3, 0), cgv::rgb(0, 1, 0));
			arrow(c, cgv::dvec3(0, 0, 0), cgv::dvec3(0, 0, 3), cgv::rgb(0, 0, 1));
		}
		for (unsigned i = 0; i < 2; ++i) {
			if (show_point[i])
				arrow(c, cgv::dvec3(0, 0, 0), cgv::dvec3((2.0f / x[i][2])*x[i]), cgv::rgb(0, 0.5f, 0.5f));
			if (show_line[i]) {
				arrow(c, cgv::dvec3(0, 0, 0), cgv::dvec3(l[i]), cgv::rgb(0.5f, 0, 0.5f));
				line(c, l[i], cgv::rgb(1, 0, 1));
			}
		}
		c.ref_surface_shader_program().disable(c);

		sphere_renderer& sr = ref_sphere_renderer(c);
		sr.set_render_style(srs);
		std::vector<cgv::vec3> P;
		std::vector<cgv::rgb> C;
		if (show_axes) {
			P.push_back(cgv::vec3(0.0f));
			C.push_back(cgv::rgb(0.5f, 0.5f, 0.5f));
		}
		for (unsigned i = 0; i < 2; ++i) {
			if (show_point[i]) {
				P.push_back(x[i]);
				C.push_back(cgv::rgb(0, 1, 1));
				P.push_back((1.0f / x[i][2])*x[i]);
				C.push_back(cgv::rgb(0, 0.5f, 0.5f));
			}
		}
		if (!P.empty()) {
			sr.set_position_array(c, P);
			sr.set_color_array(c, C);
			sr.render(c, 0, P.size());
		}
	}
	void finish_draw(context& c)
	{
		cgv::render::shader_program& prog = c.ref_surface_shader_program();
		prog.enable(c);
		prog.set_uniform(c, "map_color_to_material", 0);
		prog.set_uniform(c, "illumination_mode", 2);
		c.set_material(surface_mat);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		if (show_plane) {
			c.push_modelview_matrix();
			c.mul_modelview_matrix(translate4<double>(0, 0, 1)*scale4<double>(5, 5, 5));
			c.tesselate_unit_disk(50);
			c.pop_modelview_matrix();
		}
		for (unsigned i = 0; i < 2; ++i) {
			if (show_line_plane[i]) {
				c.push_modelview_matrix();
				cgv::dvec3 axis;
				double angle;
				compute_rotation_axis_and_angle_from_vector_pair(cgv::dvec3(0, 0, 1), l[i], axis, angle);
				c.mul_modelview_matrix(rotate4<double>(180 / M_PI * angle, axis)*scale4<double>(3, 3, 3));
				c.tesselate_unit_disk(50);
				c.pop_modelview_matrix();
			}
		}
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		c.ref_surface_shader_program().disable(c);

	}
};

factory_registration<proj_geom_demo> homo_demo_fac("proj_geom", "shortcut='Ctrl-Alt-P';menu_text='New/Demo/Project Geometry'", true);
