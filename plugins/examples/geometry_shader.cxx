#include <cgv/signal/rebind.h>
#include <cgv/base/register.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/trigger.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/attribute_array_manager.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/arrow_renderer.h>
#include <cgv/math/ftransform.h>
#include <random>

using namespace cgv::base;
using namespace cgv::data;
using namespace cgv::signal;
using namespace cgv::render;
using namespace cgv::gui;

class geometry_shader : 
	public base,
	public drawable,
	public provider
{
protected:
	shader_program prog;
	attribute_array_manager aam;
	sphere_render_style srs;
	arrow_render_style ars;
	bool show_tangents = true;

	unsigned nr_subdivisions = 32;
	int mode = 2;
	bool show_segments = true;
	float tangent_scale = 1.0f;
	float line_width = 3.0f;
	int scene_index = 0;
	unsigned nr_points = 40;
	float random_amplitude = 0.5f, damping = 0.3f, max_extent = 1.0f;

	std::vector<cgv::vec3> P[2];
	std::vector<cgv::vec3> D[2];
	std::vector<GLuint> I[2];
public:
	/// define format and texture filters in constructor
	geometry_shader() {
		srs.radius = 0.05f;
		srs.surface_color = cgv::rgb(1.0f, 0.0f, 0.0f);
		ars.radius_relative_to_length = 0.0f;
		ars.radius_lower_bound = 0.02f;
		ars.surface_color = cgv::rgb(0.6f, 0.5f, 0.0f);
		generate_simple_scene(P[0], I[0], D[0]);
		generate_random_scene(P[1], I[1], D[1]);
	}
	void compute_directions(const std::vector<cgv::vec3>& P, std::vector<cgv::vec3>& D)
	{
		D.clear();
		D.push_back(cgv::vec3(0.0f));
		for (size_t i = 2; i < P.size(); ++i)
			D.push_back(0.5f*tangent_scale * (P[i] - P[i - 2]));
		D.push_back(cgv::vec3(0.0f));
	}
	void generate_simple_scene(std::vector<cgv::vec3>& P, std::vector<GLuint>& I, std::vector<cgv::vec3>& D)
	{
		P.clear();
		I.clear();
		P.push_back(cgv::vec3(-1,0,0));
		P.push_back(cgv::vec3(0,3,0));
		P.push_back(cgv::vec3(1,3,0));
		P.push_back(cgv::vec3(2,0,0));
		I.push_back(0);
		I.push_back(1);
		I.push_back(2);
		I.push_back(3);
		compute_directions(P, D);
	}
	void generate_random_scene(std::vector<cgv::vec3>& P, std::vector<GLuint>& I, std::vector<cgv::vec3>& D)
	{
		P.clear();
		I.clear();
		std::default_random_engine E;
		std::uniform_real_distribution<float> Dist(-random_amplitude, random_amplitude);
		cgv::vec3 p(0.0f);
		cgv::vec3 v(random_amplitude, 0.0f, 0.0f);
		P.push_back(p);
		I.push_back(0);
		for (unsigned i = 0; i < nr_points; ++i) {
			p += v;
			P.push_back(p);
			I.push_back(i+1);
			v += cgv::vec3(Dist(E), Dist(E), Dist(E));
			for (int j = 0; j < 3; ++j) {
				if (p(j) > max_extent)
					v(j) -= damping;
				if (p(j) < -max_extent)
					v(j) += damping;
			}
		}
		compute_directions(P, D);
	}
	std::string get_type_name() const 
	{
		return "geometry_shader"; 
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &nr_points || member_ptr == &random_amplitude || member_ptr == &damping || member_ptr == &max_extent)
			generate_random_scene(P[1], I[1], D[1]);
		if (member_ptr == &tangent_scale) {
			compute_directions(P[0], D[0]);
			compute_directions(P[1], D[1]);
		}
		if (member_ptr >= &P[0][0] && member_ptr < &P[0].back()+1)
			compute_directions(P[0], D[0]);
		update_member(member_ptr);
		post_redraw();
	}
	bool init(context& ctx)
	{
		if (!prog.build_program(ctx, "geometry_shader.glpr")) {
			std::cout << "init failed" << std::endl;
			return false;
		}
		aam.init(ctx);
		ref_sphere_renderer(ctx, 1);
		ref_arrow_renderer(ctx, 1);
		return true;
	}
	void init_frame(context& ctx)
	{
		aam.set_attribute_array(ctx, prog.get_attribute_location(ctx, "position"), P[scene_index]);
		aam.set_indices(ctx, I[scene_index]);
	}
	void clear(context& ctx)
	{
		aam.destruct(ctx);
		ref_sphere_renderer(ctx, -1);
		ref_arrow_renderer(ctx, -1);
	}
	void draw(context& ctx)
	{
		auto& R = ref_sphere_renderer(ctx);
		R.set_render_style(srs);
		R.set_position_array(ctx, P[scene_index]);
		R.render(ctx, 0, (GLsizei)P[scene_index].size());
		glLineWidth(line_width);
		aam.enable(ctx);
		prog.enable(ctx);
		prog.set_uniform(ctx, "nr_subdivisions", nr_subdivisions);
		prog.set_uniform(ctx, "mode", mode);
		prog.set_uniform(ctx, "show_segments", show_segments);
		prog.set_uniform(ctx, "tangent_scale", tangent_scale);
		glDrawElements(GL_LINE_STRIP_ADJACENCY, GLsizei(I[scene_index].size()), GL_UNSIGNED_INT, 0);
		prog.disable(ctx);
		aam.disable(ctx);
		if (show_tangents) {
			auto& R = ref_arrow_renderer(ctx);
			R.set_render_style(ars);
			R.set_position_array(ctx, P[scene_index]);
			R.set_direction_array(ctx, D[scene_index]);
			R.render(ctx, 1, (GLsizei)P[scene_index].size()-2);
		}
	}
	void create_gui()
	{
		if (begin_tree_node("sphere style", srs)) {
			align("\a");
			add_gui("sphere style", srs);
			align("\b");
			end_tree_node(srs);
		}
		add_member_control(this, "line_width", line_width, "value_slider", "min=1;max=10;ticks=true");
		add_member_control(this, "mode", (cgv::type::DummyEnum&)mode, "dropdown", "enums='linear,bezier,hermite'");
		add_member_control(this, "nr_subdivisions", nr_subdivisions, "value_slider", "min=2;max=256;log=true;ticks=true");
		add_member_control(this, "show_segments", show_segments, "check");
		add_member_control(this, "show_tangents", show_tangents, "check");
		add_member_control(this, "tangent_scale", tangent_scale, "value_slider", "min=0.1;max=10;log=true;ticks=true");
		if (begin_tree_node("arrow style", ars)) {
			align("\a");
			add_gui("arrow style", ars);
			align("\b");
			end_tree_node(ars);
		}
		add_member_control(this, "scene", (cgv::type::DummyEnum&)scene_index, "dropdown", "enums='simple,random'");
		if (begin_tree_node("simple scene", P[0])) {
			align("\a");
			for (size_t i = 0; i < P[0].size(); ++i) {
				for (unsigned c=0; c<3; ++c)
					add_member_control(this, std::string("p") + cgv::utils::to_string(i) + "."+"xyz"[c], P[0][i][c], "value_slider", "min=-3;max=3;ticks=true");
			}
			align("\b");
			end_tree_node(P[0]);
		}
		if (begin_tree_node("random scene", P[1])) {
			align("\a");
			add_member_control(this, "nr_points", nr_points, "value_slider", "min=4;max=100;ticks=true;log=true");
			add_member_control(this, "random_amplitude", random_amplitude, "value_slider", "min=0.1;max=10;ticks=true;log=true");
			add_member_control(this, "damping", damping, "value_slider", "min=0.1;max=10;ticks=true;log=true");
			add_member_control(this, "max_extent", max_extent, "value_slider", "min=1;max=10;ticks=true;log=true");
			align("\b");
			end_tree_node(P[1]);
		}
	}
};

factory_registration<geometry_shader> fr_geosha_test("New/Render/Geometry Shader", 'L', true);