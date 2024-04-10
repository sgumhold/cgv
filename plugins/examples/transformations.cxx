#include <cgv/base/node.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/register.h>
#include <cgv/math/fvec.h>
#include <cgv/math/ftransform.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/trigger.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/point_renderer.h>
#include <cgv_gl/gl/gl.h>
#include <random>

enum RenderMode {
	RM_POINTS,
	RM_SPHERES
};
class transformations :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider
{
protected:
	std::vector<cgv::vec3> points;
	std::vector<cgv::vec3> colors;

	cgv::mat4 transformation;
	bool render_original;
	bool render_transformed;
	RenderMode render_mode;
	cgv::vec3 translation;
	float angle_x, scale_x, p_x;

	cgv::render::view* view_ptr;

	// declare render styles
	cgv::render::point_render_style point_style;
	cgv::render::sphere_render_style sphere_style;

	// declare attribute managers
	cgv::render::attribute_array_manager manager;	
public:
	/// define format and texture filters in constructor
	transformations() : cgv::base::node("transformations")
	{
		transformation.identity();
		render_original = true;
		render_transformed = true;
		render_mode = RM_POINTS;
		translation = cgv::vec3(0.0f);

		// generate random geometry
		std::default_random_engine g;
		std::uniform_real_distribution<float> d(0.0f, 1.0f);
		unsigned i;
		for (i = 0; i < 10000; ++i) {
			cgv::vec3 p(d(g), d(g), d(g));
			points.push_back(2.0f*(p - 0.5f));
			colors.push_back(p);
		}
		angle_x = 0;
		scale_x = 1.0f;
		p_x = 0.0f;
		sphere_style.radius = 0.015f;
		point_style.point_size = 1.5f;
		point_style.halo_width_in_pixel = 0.0f;
		point_style.blend_width_in_pixel = 0.0f;
		point_style.measure_point_size_in_pixel = false;
	}
	std::string get_type_name() const
	{
		return "transformations";
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &angle_x) {
			transformation = cgv::math::rotate4<float>(angle_x, cgv::vec3(1.0f, 0.0f, 0.0f));
			update_all_members();
		}
		else if (member_ptr == &scale_x) {
			transformation = cgv::math::scale4<float>(scale_x, 1.0f, 1.0f);
			update_all_members();
		}
		else if (member_ptr == &p_x) {
			transformation(3,0) = p_x;
			update_member(&transformation(3,0));
		}
		update_member(member_ptr);
		post_redraw();
	}
	bool init(cgv::render::context& ctx)
	{
		if ((view_ptr = find_view_as_node())) {
//			view_ptr->set_focus(0.0, 0.0, 0.0);
//			view_ptr->set_y_extent_at_focus(2.0);
		}
		ctx.set_bg_clr_idx(4);

		// create attribute managers
		if (!manager.init(ctx))
			return false;
		// increase reference counts of used renderer singletons
		cgv::render::ref_point_renderer(ctx, 1);
		auto& sr = cgv::render::ref_sphere_renderer  (ctx, 1);
		sr.enable_attribute_array_manager(ctx, manager);
		sr.set_position_array(ctx, points);
		sr.set_color_array(ctx, colors);
		sr.disable_attribute_array_manager(ctx, manager);
		return true;
	}
	void draw_points(cgv::render::context& ctx)
	{
		cgv::render::renderer& r = (render_mode == RM_POINTS ? 
			static_cast<cgv::render::renderer&>(cgv::render::ref_point_renderer(ctx)) : 
			static_cast<cgv::render::renderer&>(cgv::render::ref_sphere_renderer(ctx)));
		r.enable_attribute_array_manager(ctx, manager);
		r.set_render_style(render_mode == RM_POINTS ?
			static_cast<cgv::render::render_style&>(point_style) :
			static_cast<cgv::render::render_style&>(sphere_style));
		if (r.validate_and_enable(ctx)) {
			glDrawArrays(GL_POINTS, 0, GLsizei(points.size()));
			r.disable(ctx);
		}
		r.disable_attribute_array_manager(ctx, manager);
	}
	void draw(cgv::render::context& ctx)
	{
		cgv::render::ref_point_renderer(ctx).set_reference_point_size(0.02f);
		cgv::render::ref_point_renderer(ctx).set_y_view_angle(float(view_ptr->get_y_view_angle()));

		if (render_original) {
			ctx.push_modelview_matrix();
			ctx.mul_modelview_matrix(cgv::math::translate4<float>(translation));
			draw_points(ctx);
			ctx.pop_modelview_matrix();
		}
		if (render_transformed) {
			ctx.push_modelview_matrix();
			ctx.mul_modelview_matrix(transformation);
			ctx.mul_modelview_matrix(cgv::math::translate4<float>(translation));
			draw_points(ctx);
			ctx.pop_modelview_matrix();
		}
	}
	void clear(cgv::render::context& ctx)
	{
		// clear attribute managers
		manager.destruct(ctx);

		// decrease reference counts of used renderer singletons
		cgv::render::ref_sphere_renderer  (ctx, -1);
	}
	void identity_callback()
	{
		transformation = cgv::math::identity4<float>();
		update_all_members();
		post_redraw();
	}
	void create_gui()
	{
		add_decorator("transformations", "heading");
		add_member_control(this, "pretranslate_x", translation(0), "value_slider", "min=-2;max=2;ticks=true");
		add_member_control(this, "pretranslate_y", translation(1), "value_slider", "min=-2;max=2;ticks=true");
		add_member_control(this, "pretranslate_z", translation(2), "value_slider", "min=-2;max=2;ticks=true");
		add_member_control(this, "render_original", render_original, "check");
		add_member_control(this, "render_transformed", render_transformed, "check");
		add_member_control(this, "render_mode", render_mode, "dropdown", "enums='POINTS,SPHERES'");
		connect_copy(add_button("identity")->click, cgv::signal::rebind(this, &transformations::identity_callback));
		add_member_control(this, "angle_x", angle_x, "value_slider", "min=-180;max=180;ticks=true");
		add_member_control(this, "scale_x", scale_x, "value_slider", "min=-10;max=10;log=true;ticks=true");
		add_member_control(this, "p_x", p_x, "value_slider", "min=-10;max=10;log=true;ticks=true");
		for (unsigned i = 0; i < 4; ++i) {
			for (unsigned j = 0; j < 4; ++j)
				add_member_control(this, (i == 0 && j == 0) ? "T" : "", transformation(i, j), "value", "w=50", j == 3 ? "\n" : " ");
			for (unsigned j = 0; j < 4; ++j)
				add_member_control(this, "", transformation(i, j), "slider", "min=-1;max=1;w=50;ticks=true", j == 3 ? "\n" : " ");
		}
		if (begin_tree_node("Point Rendering", point_style, false)) {
			align("\a");
			add_gui("point_style", point_style);
			align("\b");
			end_tree_node(point_style);
		}
		if (begin_tree_node("Sphere Rendering", sphere_style, false)) {
			align("\a");
			add_gui("sphere_style", sphere_style);
			align("\b");
			end_tree_node(sphere_style);
		}
	}
};

cgv::base::factory_registration<transformations> fr_transformations("New/Demo/Transformations", 'T', true);