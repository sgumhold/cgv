#include "viewer.h"

viewer::viewer() {
	
	set_name("Viewer");

	view_ptr = nullptr;

	use_radii = true;
	use_position_tangents = true;
	use_radius_tangents = true;
	use_colors = true;

	spline_tubes_style.radius = 0.2f;
}

void viewer::clear(cgv::render::context& ctx) {

	ref_spline_tube_renderer(ctx, -1);
}

bool viewer::self_reflect(cgv::reflect::reflection_handler& rh) {

	return false;
}

bool viewer::handle(cgv::gui::event& e) {

	return false;
}

void viewer::on_set(void* member_ptr) {

	update_member(member_ptr);
	post_redraw();
}

bool viewer::init(cgv::render::context& ctx) {

	ref_spline_tube_renderer(ctx, 1);
	
	ctx.set_bg_clr_idx(0);
	return true;
}

void viewer::init_frame(cgv::render::context& ctx) {

	if(!view_ptr) {
		if(view_ptr = find_view_as_node()) {
			view_ptr->set_eye_keep_view_angle(cgv::dvec3(2.5f, 0.0f, 2.5f));
		}
	}
}

void viewer::draw(cgv::render::context& ctx) {

	std::vector<cgv::vec3> positions = { {-2.f, 0.f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f, -2.f} };
	std::vector<float> radii = { 0.2f, 0.1f, 0.1f, 0.2f };
	std::vector<cgv::vec4> tangents(4, cgv::vec4(0.0f));
	
	if(use_position_tangents) {
		tangents[0] = cgv::vec4(0.0f, 4.0f, 0.0f, 0.0f);
		tangents[1] = cgv::vec4(0.0f, -4.0f, 0.0f, 0.0f);
		tangents[2] = cgv::vec4(0.0f, -4.0f, 0.0f, 0.0f);
		tangents[3] = cgv::vec4(0.0f, 4.0f, 0.0f, 0.0f);
	}

	if(use_radius_tangents) {
		tangents[0].w() = -0.2f;
		tangents[1].w() = 0.2f;
		tangents[2].w() = 0.2f;
		tangents[3].w() = 0.4f;
	}
	std::vector<cgv::rgb> colors = { {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f} };
	spline_tube_renderer& str = ref_spline_tube_renderer(ctx);
	str.set_render_style(spline_tubes_style);
	str.set_position_array(ctx, positions);
	if (use_radii)
		str.set_radius_array(ctx, radii);
	else
		str.remove_radius_array(ctx);
	if (use_radius_tangents || use_position_tangents)
		str.set_tangent_array(ctx, tangents);
	else
		str.remove_tangent_array(ctx);
	if (use_colors)
		str.set_color_array(ctx, colors);
	else
		str.remove_color_array(ctx);
	
	str.render(ctx, 0, positions.size());
}

void viewer::create_gui() {
	add_decorator("Viewer", "heading", "level=2");

	if(begin_tree_node("Spline Tube Rendering", spline_tubes_style, true)) {
		align("\a");
		add_gui("spline_tubes_style", spline_tubes_style);
		align("\b");
		end_tree_node(spline_tubes_style);
	}

	add_member_control(this, "Use Radii", use_radii, "check");
	add_member_control(this, "Use Position Tangents", use_position_tangents, "check");
	add_member_control(this, "Use Radius Tangents", use_radius_tangents, "check");
	add_member_control(this, "Use Colors", use_colors, "check");
}

#include "lib_begin.h"

#include <cgv/base/register.h>

extern CGV_API cgv::base::object_registration<viewer> viewer_reg("viewer");

#ifdef CGV_FORCE_STATIC
cgv::base::registration_order_definition ro_def("stereo_view_interactor;viewer");
#endif

