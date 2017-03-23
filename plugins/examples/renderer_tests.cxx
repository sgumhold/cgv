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
	typedef cgv::media::color<float,cgv::media::RGB,cgv::media::OPACITY> clr4;
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

	bool sort_points;
	bool interleaved_mode;
	std::vector<vertex> vertices;

	bool use_group_colors;
	bool use_group_transformations;
	std::vector<vec4> group_colors;
	std::vector<vec3> group_translations;
	std::vector<vec4> group_rotations;
	std::vector<vec3> sizes;
	std::vector<unsigned> indices;
	RenderMode mode;
	cgv::render::view* view_ptr;
	cgv::render::point_render_style point_style;
	cgv::render::point_renderer p_renderer;
public:
	/// define format and texture filters in constructor
	renderer_tests() : cgv::base::node("renderer_test")
	{
		sort_points = true;
		interleaved_mode = false;
		// generate random geometry
		std::default_random_engine g;
		std::uniform_real_distribution<float> d(0.0f, 1.0f);
		unsigned i;
		for (i = 0; i < 1000000; ++i) {
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
		use_group_colors = true;
		use_group_transformations = true;
		for (i = 0; i < 8; ++i) {
			group_colors.push_back(vec4((i & 1) != 0 ? 1.0f : 0.0f, (i & 2) != 0 ? 1.0f : 0.0f, (i & 4) != 0 ? 1.0f : 0.0f, 1.0f));
			group_translations.push_back(vec3(0, 0, 0));
			group_rotations.push_back(vec4(0, 0, 0, 1));
		}
		mode = RM_POINTS;
		point_style.point_size = 15;
		point_style.measure_point_size_in_pixel = false;
		point_style.illumination_mode = cgv::render::IM_TWO_SIDED;

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
		p_renderer.set_group_translations(ctx, group_translations);
		p_renderer.set_group_rotations(ctx, group_rotations);
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
		p_renderer.enable(ctx, point_style, 0.0001f, float(view_ptr->get_y_view_angle()), true, true, false, use_group_colors, use_group_transformations);
		if (sort_points) {
			indices.resize(points.size());
			for (unsigned i = 0; i < indices.size(); ++i)
				indices[i] = i;
			struct sort_pred {
				const std::vector<vec3>& points;
				const vec3& view_dir;
				bool operator () (GLint i, GLint j) const {
					return dot(points[i], view_dir) > dot(points[j], view_dir);
				}
				sort_pred(const std::vector<vec3>& _points, const vec3& _view_dir) : points(_points), view_dir(_view_dir) {}
			};
			vec3 view_dir = view_ptr->get_view_dir();
			std::sort(indices.begin(), indices.end(), sort_pred(points, view_dir));
			glDepthFunc(GL_ALWAYS);
			glDrawElements(GL_POINTS, points.size(), GL_UNSIGNED_INT, &indices.front());
			glDepthFunc(GL_LESS);
		}
		else
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
		add_member_control(this, "sort_points", sort_points, "check");
		add_member_control(this, "use_group_colors", use_group_colors, "check");
		add_member_control(this, "use_group_transformations", use_group_transformations, "check");
		add_member_control(this, "interleaved_mode", interleaved_mode, "check");
		if (begin_tree_node("group colors", use_group_colors, false)) {
			align("\a");
			for (unsigned i = 0; i < group_colors.size(); ++i) {
				add_member_control(this, std::string("C") + cgv::utils::to_string(i), reinterpret_cast<clr4&>(group_colors[i]));
			}
			add_gui("point_style", point_style);
			align("\b");
		}
		if (begin_tree_node("group transformations", use_group_transformations, true)) {
			align("\a");
			for (unsigned i = 0; i < group_translations.size(); ++i) {
				add_gui(std::string("T") + cgv::utils::to_string(i), group_translations[i]);
				add_gui(std::string("Q") + cgv::utils::to_string(i), group_rotations[i], "direction");
			}
			add_gui("point_style", point_style);
			align("\b");
		}
		if (begin_tree_node("point style", point_style, true)) {
			align("\a");
			add_gui("point_style", point_style);
			align("\b");
			end_tree_node(point_style);
		}
	}
};

cgv::base::factory_registration<renderer_tests> fr_renderer_test("new/render_test", 'K', true);