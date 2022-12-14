#include <cgv/base/node.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/register.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/a_buffer.h>
#include <random>

using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::render::gl;

class a_buffer_test :
	public node,
	public drawable,
	public provider
{
protected:
	// rendering configuration
	bool use_a_buffer_for_triangles;
	bool use_a_buffer_for_spheres;
	bool show_triangles;
	bool show_spheres;

	// triangle geometry
	struct Vertex {
		vec3 p;
		rgba c;
		vec3 n;
	};
	std::vector<Vertex> V;

	// sphere geometry
	std::vector<vec4> S;
	std::vector<rgba> C;

	// render objects
	shader_define_map triangle_defines;

	vertex_buffer triangle_vbo;
	attribute_array_binding triangle_aab;
	shader_program triangle_prog;

	sphere_render_style srs;
	attribute_array_manager sphere_aam;
	sphere_renderer sr;
	cgv::render::view* view_ptr = 0;

	// a buffer instance
	a_buffer a_b;
	size_t node_cnt;
	float fullness;

	void create_triangles(size_t count, float delta)
	{
		std::default_random_engine e;
		std::uniform_real_distribution<float> d(-1.0f, 1.0f);
		for (size_t i = 0; i < count; ++i) {
			vec3 p0(d(e), d(e), d(e));
			vec3 e1(d(e), d(e), d(e));
			vec3 e2(d(e), d(e), d(e));
			vec3 p1 = p0 + delta * e1;
			vec3 p2 = p0 + delta * e2;
			vec3 n = cross(e1, e2);
			n.normalize();
			rgba c(0.5f * d(e) + 0.5f, 0.25f * d(e) + 0.5f, 0.25f * d(e) + 0.5f, 0.25f * d(e) + 0.65f);
			V.push_back({ p0, c, n });
			V.push_back({ p1, c, n });
			V.push_back({ p2, c, n });
		}
	}
	void create_spheres(size_t count, float radius)
	{
		std::default_random_engine e;
		std::uniform_real_distribution<float> d(-1.0f, 1.0f);
		for (size_t i = 0; i < count; ++i) {
			vec4 s(d(e), d(e), d(e), radius * (d(e) + 1.5f));
			rgba c(0.5f * d(e) + 0.5f, 0.25f * d(e) + 0.5f, 0.25f * d(e) + 0.5f, 0.25f * d(e) + 0.65f);
			S.push_back(s);
			C.push_back(c);
		}
	}
	shader_define_map build_defines(bool use_a_buffer)
	{
		shader_define_map defines;
		if (use_a_buffer) {
			defines["USE_A_BUFFER"] = "1";
			a_b.update_defines(defines);
		}
		return defines;
	}

public:
	a_buffer_test() : a_b(32, 64)
	{
		set_name("a_buffer_test");
		node_cnt = 0;
		fullness = 0;
		show_triangles = true;
		use_a_buffer_for_triangles = true;
		show_spheres = true;
		use_a_buffer_for_spheres = true;
		srs.map_color_to_material = CM_COLOR_AND_OPACITY;
	}
	void on_set(void* member_ptr)
	{
		update_member(member_ptr);
		post_redraw();
	}
	std::string get_type_name() const { return "a_buffer_test"; }
	void create_gui()
	{
		add_decorator("a_buffer", "heading", "level=1");
		add_member_control(this, "fragments_per_pixel", a_b.fragments_per_pixel, "value_slider", "min=1;max=64;log=true;ticks=true");
		add_member_control(this, "nodes_per_pixel", a_b.nodes_per_pixel, "value_slider", "min=1;max=64;log=true;ticks=true");
		add_view("node_cnt", node_cnt);
		add_view("fullness", fullness);
		add_decorator("internal", "heading", "level=2");
		add_member_control(this, "depth_tex_unit", a_b.ref_depth_tex_unit(), "value_slider", "min=0;max=7;ticks=true");
		add_member_control(this, "node_counter_binding_point", a_b.ref_node_counter_binding_point(), "value_slider", "min=0;max=7;ticks=true");
		add_member_control(this, "head_pointers_binding_point", a_b.ref_head_pointers_binding_point(), "value_slider", "min=0;max=7;ticks=true");
		add_member_control(this, "nodes_binding_point", a_b.ref_nodes_binding_point(), "value_slider", "min=0;max=7;ticks=true");
		add_decorator("render config", "heading", "level=1");
		add_member_control(this, "show_triangles", show_triangles, "check", "shortcut='T'");
		add_member_control(this, "use_a_buffer_for_triangles", use_a_buffer_for_triangles, "check", "shortcut='A'");
		add_member_control(this, "show_spheres", show_spheres, "check", "shortcut='S'");
		add_member_control(this, "use_a_buffer_for_spheres", use_a_buffer_for_spheres, "check", "shortcut='B'");
		add_decorator("sphere style", "heading", "level=1");
		add_gui("sphere style", srs);
	}
	bool init(context& ctx)
	{
		// init the a buffer
		if (!a_b.init(ctx))
			return false;
		// construct triangle defines
		triangle_defines = build_defines(use_a_buffer_for_triangles);	
		// and provide them to build program function
		if (!triangle_prog.build_program(ctx, "default_surface.glpr", true, triangle_defines))
			return false;
		create_triangles(200, 0.8f);
		triangle_vbo.create(ctx, V);
		triangle_aab.create(ctx);
		triangle_aab.set_attribute_array(ctx, triangle_prog.get_position_index(), get_element_type(V.front().p), triangle_vbo, 0, V.size(), sizeof(Vertex));
		triangle_aab.set_attribute_array(ctx, triangle_prog.get_color_index(), get_element_type(V.front().c), triangle_vbo, offsetof(Vertex, c), V.size(), sizeof(Vertex));
		triangle_aab.set_attribute_array(ctx, triangle_prog.get_normal_index(), get_element_type(V.front().n), triangle_vbo, offsetof(Vertex, n), V.size(), sizeof(Vertex));

		create_spheres(60, 0.2f);
		// configure defines before init
		sr.ref_defines() = build_defines(use_a_buffer_for_spheres);
		sr.init(ctx);
		sr.set_render_style(srs);
		sphere_aam.init(ctx);
		sr.enable_attribute_array_manager(ctx, sphere_aam);
		sr.set_sphere_array(ctx, S);
		sr.set_color_array(ctx, C);
		return true;
	}
	void destruct(context& ctx)
	{
		triangle_vbo.destruct(ctx);
		triangle_aab.destruct(ctx);
		triangle_prog.destruct(ctx);

		sr.disable_attribute_array_manager(ctx, sphere_aam);
		sr.clear(ctx);
		sphere_aam.destruct(ctx);

		a_b.destruct(ctx);
	}
	void init_frame(context& ctx)
	{
		if (!view_ptr)
			view_ptr = find_view_as_node();
		// rebuild triangle program if defines changes
		shader_define_map new_triangle_defines = build_defines(use_a_buffer_for_triangles);
		if (new_triangle_defines != triangle_defines) {
			triangle_defines = new_triangle_defines;
			if (triangle_prog.is_created())
				triangle_prog.destruct(ctx);
			triangle_prog.build_program(ctx, "default_surface.glpr", true, triangle_defines);
		}
		// update defines for sphere renderer
		sr.ref_defines() = build_defines(use_a_buffer_for_spheres);
		if (view_ptr)
			sr.set_y_view_angle(float(view_ptr->get_y_view_angle()));

		// init frame for a buffer
		if (use_a_buffer_for_triangles || use_a_buffer_for_spheres)
			a_b.init_frame(ctx);
	}
	void draw_triangles(context& ctx, shader_program& prog, attribute_array_binding& aab)
	{
		prog.set_uniform(ctx, "culling_mode", 0);
		prog.set_uniform(ctx, "map_color_to_material", 15);
		prog.set_uniform(ctx, "illumination_mode", 2);

		glDisable(GL_CULL_FACE);
		prog.enable(ctx);
		aab.enable(ctx);
		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)V.size());
		aab.disable(ctx);
		prog.disable(ctx);
		glEnable(GL_CULL_FACE);
	}
	void draw(context& ctx)
	{
		if (show_triangles && !use_a_buffer_for_triangles)
			draw_triangles(ctx, triangle_prog, triangle_aab);
		if (show_spheres && !use_a_buffer_for_spheres)
			sr.render(ctx, 0, S.size());
	}
	void finish_frame(context& ctx)
	{
		if (!(show_triangles && use_a_buffer_for_triangles) && !(show_spheres && use_a_buffer_for_spheres))
			return;
		if (show_triangles && use_a_buffer_for_triangles) {
			a_b.enable(ctx, triangle_prog);
			draw_triangles(ctx, triangle_prog, triangle_aab);
			node_cnt = a_b.disable(ctx);
		}
		if (show_spheres && use_a_buffer_for_spheres) {
			if (sr.enable(ctx)) {
				a_b.enable(ctx, sr.ref_prog());
				sr.draw(ctx, 0, S.size());
				node_cnt = a_b.disable(ctx);
				sr.disable(ctx);
			}
		}
		a_b.finish_frame(ctx);
		update_member(&node_cnt);
		fullness = float(node_cnt) / (ctx.get_width() * ctx.get_height() * a_b.nodes_per_pixel);
		update_member(&fullness);
	}
};

object_registration<a_buffer_test> a_buffer_reg("");
