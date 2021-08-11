#include <cgv/base/node.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/register.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/utils/file.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/a_buffer.h>
#include <string>
#include <fstream>
#include <random>
#include <cgv/utils/stopwatch.h>

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
	bool animate;
	bool use_a_buffer;
	int time_step;
	size_t nr_time_steps;

	struct Vertex {
		vec3 p;
		rgba c;
		vec3 n;
	};
	std::vector<Vertex> V;

	std::vector<vec4> S;
	std::vector<rgb> C;

	sphere_render_style srs;
	bool show_spheres;

	shader_program surface_prog;
	shader_define_map defines;
	
	a_buffer a_b;
	size_t node_cnt;
	vertex_buffer vbo;
	attribute_array_binding a_buffer_aab;
	attribute_array_binding aab;
	attribute_array_manager aam;

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
			rgba c(0.5f * d(e) + 0.5f, 0.25f * d(e) + 0.5f, 0.25f * d(e) + 0.5f);
			S.push_back(s);
			C.push_back(c);
		}
	}
	shader_define_map build_defines()
	{
		shader_define_map defines;
		defines["USE_A_BUFFER"] = use_a_buffer ? "1" : "0";
		return defines;
	}

public:
	a_buffer_test()
	{
		set_name("a_buffer_test");
		animate = false;
		time_step = 0;
		nr_time_steps = 20;
		node_cnt = 0;
		use_a_buffer = true;
		show_spheres = true;
		srs.map_color_to_material = CM_COLOR_AND_OPACITY;
		connect(get_animation_trigger().shoot, this, &a_buffer_test::timer_event);
	}
	void timer_event(double, double)
	{
		if (!animate)
			return;
		++nr_time_steps;
		on_set(&time_step);
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
		add_view("node_cnt", node_cnt);
		add_member_control(this, "use_a_buffer", use_a_buffer, "check");
		add_member_control(this, "show_spheres", show_spheres, "check");
		add_member_control(this, "time_step", time_step, "value_slider", "min=0;ticks=true")->set("max", nr_time_steps - 1);
	}
	bool init(context& ctx)
	{
		defines = build_defines();
		if (!a_b.init(ctx))
			return false;
		if (!surface_prog.build_program(ctx, "a_buffer_surface.glpr", true, defines))
			return false;
		create_triangles(200, 0.8f);
		create_spheres(60, 0.2f);

		vbo.create(ctx, V);
		shader_program& prog = ctx.ref_surface_shader_program();
		aab.create(ctx);
		aab.set_attribute_array(ctx, prog.get_position_index(), get_element_type(V.front().p), vbo, 0, V.size(), sizeof(Vertex));
		aab.set_attribute_array(ctx, prog.get_color_index(), get_element_type(V.front().c), vbo, offsetof(Vertex, c), V.size(), sizeof(Vertex));
		aab.set_attribute_array(ctx, prog.get_normal_index(), get_element_type(V.front().n), vbo, offsetof(Vertex, n), V.size(), sizeof(Vertex));

		a_buffer_aab.create(ctx);
		a_buffer_aab.set_attribute_array(ctx, surface_prog.get_position_index(), get_element_type(V.front().p), vbo, 0, V.size(), sizeof(Vertex));
		a_buffer_aab.set_attribute_array(ctx, surface_prog.get_color_index(), get_element_type(V.front().c), vbo, offsetof(Vertex, c), V.size(), sizeof(Vertex));
		a_buffer_aab.set_attribute_array(ctx, surface_prog.get_normal_index(), get_element_type(V.front().n), vbo, offsetof(Vertex, n), V.size(), sizeof(Vertex));

		aam.init(ctx);
		auto& sr = ref_sphere_renderer(ctx, 1);
		sr.enable_attribute_array_manager(ctx, aam);
		sr.set_sphere_array(ctx, S);
		sr.set_color_array(ctx, C);
		sr.disable_attribute_array_manager(ctx, aam);
		return true;
	}
	void destruct(context& ctx)
	{
		ref_sphere_renderer(ctx, -1);
		a_b.destruct(ctx);
	}
	void init_frame(context& ctx)
	{
		shader_define_map new_defines = build_defines();
		if (new_defines != defines) {
			defines = new_defines;
			if (surface_prog.is_created())
				surface_prog.destruct(ctx);
			surface_prog.build_program(ctx, "a_buffer_surface.glpr", true, defines);
		}
		if (use_a_buffer)
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
		if (show_spheres) {
			// draw opaque spheres
			auto& sr = ref_sphere_renderer(ctx);
			sr.set_render_style(srs);
			sr.enable_attribute_array_manager(ctx, aam);
			sr.render(ctx, 0, S.size());
			sr.disable_attribute_array_manager(ctx, aam);
		}
	}
	void finish_frame(context& ctx)
	{
		if (use_a_buffer) {
			a_b.enable(ctx, surface_prog);
			draw_triangles(ctx, surface_prog, a_buffer_aab);
			node_cnt = a_b.disable(ctx);
			update_member(&node_cnt);
			a_b.finish_frame(ctx);
		}
		else {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			draw_triangles(ctx, ctx.ref_surface_shader_program(), aab);
			glDisable(GL_BLEND);
		}
	}
};

object_registration<a_buffer_test> a_buffer_reg("");
