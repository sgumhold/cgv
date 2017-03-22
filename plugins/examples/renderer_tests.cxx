#include <cgv/signal/rebind.h>
#include <cgv/base/node.h>
#include <cgv/base/register.h>
#include <cgv/math/fvec.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/trigger.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/point_renderer.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/base/find_action.h>
#include <cgv/render/view.h>
#include <random>

class renderer_tests :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider
{
public:
	typedef cgv::math::fvec<float, 3> vec3;
	typedef cgv::math::fvec<float, 4> vec4;
	enum RenderMode { RM_POINTS, RM_BOXES, RM_SPHERES };
	struct vertex {
		vec3 point;
		vec3 normal;
		vec4 color;
	};
protected:
	std::vector<vec3> points;
	std::vector<unsigned> group_indices;
	std::vector<vec3> normals;
	std::vector<vec4> colors;

	bool interleaved_mode;
	std::vector<vertex> vertices;

	bool use_group_colors;
	std::vector<vec4> group_colors;
	std::vector<vec3> sizes;
	RenderMode mode;
	cgv::render::view* view_ptr;
	cgv::render::point_render_style point_style;
	cgv::render::point_renderer p_renderer;
public:
	/// define format and texture filters in constructor
	renderer_tests() : cgv::base::node("renderer_test")
	{
		interleaved_mode = false;
		// generate random geometry
		std::default_random_engine g;
		std::uniform_real_distribution<float> d(0.0f, 1.0f);
		unsigned i;
		for (i = 0; i < 1000; ++i) {
			points.push_back(vec3(d(g), d(g), d(g)));
			unsigned group_index = 0;
			if (points.back()[0] > 0.5f)
				group_index += 1;
			if (points.back()[1] > 0.5f)
				group_index += 2;
			if (points.back()[2] > 0.5f)
				group_index += 4;
			group_indices.push_back(group_index);
			normals.push_back(points.back() - vec3(0.5f, 0.5f, 0.5f));
			normals.back().normalize();
			colors.push_back(vec4(d(g), d(g), d(g), 1.0f));
			vertices.push_back(vertex());
			vertices.back().point = points.back();
			vertices.back().normal = normals.back();
			vertices.back().color = colors.back();
			sizes.push_back(vec3(0.01f*d(g) + 0.001f, 0.01f*d(g) + 0.001f, 0.01f*d(g) + 0.001f));
		}
		use_group_colors = false;
		for (i = 0; i < 8; ++i) 
			group_colors.push_back(vec4((i & 1) != 0 ? 1.0f : 0.0f, (i & 2) != 0 ? 1.0f : 0.0f, (i & 4) != 0 ? 1.0f : 0.0f, 1.0f));

		mode = RM_POINTS;
		point_style.point_size = 3;
	}
	std::string get_type_name() const
	{
		return "renderer_tests";
	}
	
	void on_set(void* member_ptr)
	{
		update_member(member_ptr);
		post_redraw();
	}

	bool init(cgv::render::context& ctx)
	{
		std::vector<cgv::render::view*> views;
		cgv::base::find_interface<cgv::render::view>(cgv::base::base_ptr(this), views);
		if (views.empty())
			return false;
		view_ptr = views[0];
		return p_renderer.init(ctx);
	}
	void draw(cgv::render::context& ctx)
	{
		p_renderer.set_group_colors(ctx, group_colors);
		if (interleaved_mode) {
			p_renderer.ref_prog().set_attribute_array(ctx, "position", &vertices.front().point, vertices.size(), sizeof(vertex));
			p_renderer.ref_prog().set_attribute_array(ctx, "normal", &vertices.front().normal, vertices.size(), sizeof(vertex));
			p_renderer.ref_prog().set_attribute_array(ctx, "color", &vertices.front().color, vertices.size(), sizeof(vertex));
		}
		else {
			p_renderer.ref_prog().set_attribute(ctx, "position", points);
			p_renderer.ref_prog().set_attribute(ctx, "normal", normals);
			p_renderer.ref_prog().set_attribute(ctx, "color", colors);
		}
		p_renderer.set_group_index_attribute(ctx, group_indices);
		p_renderer.enable(ctx, point_style, 0.003f, float(view_ptr->get_y_view_angle()), true, true, false, use_group_colors);
		glDrawArrays(GL_POINTS, 0, points.size());
		p_renderer.disable(ctx, point_style, true);
	}
	void clear(cgv::render::context& ctx)
	{
		p_renderer.clear(ctx);
	}
	void create_gui()
	{
		add_decorator("renderer tests", "heading");
		add_member_control(this, "mode", mode, "dropdown", "enums='points,boxes,spheres'");
		add_member_control(this, "use_group_colors", use_group_colors, "check");
		add_member_control(this, "interleaved_mode", interleaved_mode, "check");
		add_gui("point_style", point_style);
	}
};

cgv::base::factory_registration<renderer_tests> fr_renderer_test("new/render_test", 'K', true);