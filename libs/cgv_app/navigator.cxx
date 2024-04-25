#include "navigator.h"

#include <cgv/gui/animate.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/math/ftransform.h>
#include <cgv_gl/gl/gl.h>

using namespace cgv::render;

namespace cgv {
namespace app {

navigator::navigator() {
	
	set_name("Navigator");
	gui_options.allow_stretch = false;

	view_ptr = nullptr;
	navigator_eye_pos = vec3(0.0f, 0.0f, 2.5f);

	check_for_click = -1;

	layout_size = 150;

	set_alignment(AO_END, AO_END);
	set_stretch(SO_NONE);
	set_margin(ivec2(0));
	overlay::set_size(ivec2(layout_size));
	
	show_box = true;
	show_wireframe = true;
	use_perspective = true;
	hit_axis = 0;

	box_data.style.default_extent = vec3(1.0f);
	box_data.style.map_color_to_material = CM_COLOR_AND_OPACITY;
	box_data.style.surface_color = rgb(0.5f);

	box_data.style.illumination_mode = IM_TWO_SIDED;
	box_data.style.culling_mode = CM_OFF;
	box_data.style.material.set_diffuse_reflectance(rgb(0.5f));
	box_data.style.material.set_emission(rgb(0.05f));
	box_data.style.surface_opacity = 0.35f;

	box_wire_data.style.default_color = rgb(0.75f);
	
	sphere_data.style.illumination_mode = IM_OFF;
	sphere_data.style.radius = 0.04f;
	sphere_data.style.surface_color = rgb(0.5f);

	arrow_data.style.illumination_mode = IM_OFF;
	arrow_data.style.radius_relative_to_length = 0.04f;
	arrow_data.style.head_length_mode = AHLM_RELATIVE_TO_LENGTH;
	arrow_data.style.head_length_relative_to_length = 0.3f;
	arrow_data.style.head_radius_scale = 2.5f;

	rectangle_data.style.illumination_mode = IM_OFF;
	rectangle_data.style.map_color_to_material = CM_COLOR_AND_OPACITY;
	rectangle_data.style.surface_color = rgb(0.05f, 0.25f, 1.0f);
	rectangle_data.style.surface_opacity = 0.75f;
	rectangle_data.style.pixel_blend = 0.0f;
	rectangle_data.style.percentual_border_width = 0.111111f;
	rectangle_data.style.default_border_color = rgba(0.05f, 0.15f, 0.8f, 0.75f);
}

void navigator::clear(context& ctx) {

	blit_canvas.destruct(ctx);
	fbc.destruct(ctx);

	arrow_data.destruct(ctx);
	box_data.destruct(ctx);
	box_wire_data.destruct(ctx);
	rectangle_data.destruct(ctx);
	sphere_data.destruct(ctx);

	box_renderer.clear(ctx);
}

bool navigator::self_reflect(cgv::reflect::reflection_handler& _rh) {
	return _rh.reflect_member("layout_size", layout_size);
}

bool navigator::handle_event(cgv::gui::event& e) {

	// return true if the event gets handled and stopped here or false if you want to pass it to the next plugin
	unsigned et = e.get_kind();
	unsigned char modifiers = e.get_modifiers();

	if(et == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = (cgv::gui::mouse_event&) e;
		cgv::gui::MouseAction ma = me.get_action();

		int last_hit_axis = hit_axis;
		hit_axis = 0;

		if(get_context()) {
			context& ctx = *get_context();

			if(ma == cgv::gui::MA_LEAVE) {
				hit_axis = 0;
			} else {
				ivec2 mpos(static_cast<int>(me.get_x()), static_cast<int>(me.get_y()));

				mpos = get_local_mouse_pos(mpos);
				vec2 window_coord = vec2(mpos) * vec2(2.0f) / get_rectangle().size - vec2(1.0f);

				vec3 origin = vec3(window_coord, navigator_eye_pos.z());
				vec3 direction = vec3(0.0f, 0.0f, -1.0f);

				if(use_perspective) {
					mat4 MVP = get_projection_matrix() * get_view_matrix(ctx);

					vec4 world_coord(window_coord.x(), window_coord.y(), 1.0f, 1.0f);
					world_coord = inv(MVP) * world_coord;
					world_coord /= world_coord.w();

					vec3 origin = navigator_eye_pos;
					vec3 direction = normalize(vec3(world_coord) - origin);
				}

				mat4 IM = inv(get_model_matrix(ctx));

				origin = vec3(IM * vec4(origin, 1.0f));
				direction = vec3(IM * vec4(direction, 0.0f));

				float t = std::numeric_limits<float>::max();
				if(intersect_box(origin, direction, t)) {
					vec3 hit_pos = origin + t * direction;

					unsigned mi = cgv::math::max_index(cgv::math::abs(hit_pos));

					hit_axis = static_cast<int>(mi) + 1;
					if(hit_pos[mi] < 0.0f)
						hit_axis = -hit_axis;
				}
			}
		
			if(last_hit_axis != hit_axis)
				post_redraw();
		}

		switch(ma) {
		case cgv::gui::MA_PRESS:
			if(me.get_button() == cgv::gui::MB_LEFT_BUTTON) {
				check_for_click = me.get_time();
				return true;
			}
			break;
		case cgv::gui::MA_RELEASE:
			if(check_for_click != -1) {
				double dt = me.get_time() - check_for_click;
				if(dt < 0.2) {
					if(hit_axis != 0) {
						int axis_idx = abs(hit_axis) - 1;

						vec3 view_dir(0.0f);
						view_dir[axis_idx] = hit_axis < 0.0f ? -1.0f : 1.0f;

						if(view_ptr) {
							vec3 focus = view_ptr->get_focus();
							float dist = (focus - view_ptr->get_eye()).length();
							
							vec3 view_up_dir(0.0f, 1.0f, 0.0f);
							if(axis_idx == 1)
								view_up_dir = vec3(0.0f, 0.0f, hit_axis < 0 ? 1.0f : -1.0f);
							
							dvec3 axis;
							double angle;
							view_ptr->compute_axis_and_angle(-view_dir, view_up_dir, axis, angle);
							
							cgv::gui::animate_with_axis_rotation(view_ptr->ref_view_dir(), axis, angle, 0.5)->set_base_ptr(this);
							cgv::gui::animate_with_axis_rotation(view_ptr->ref_view_up_dir(), axis, angle, 0.5)->set_base_ptr(this);

							post_redraw();
							return true;
						}
					}
				}
			}
			return true;
			break;
		}

		return false;
	} else {
		return false;
	}
}

void navigator::on_set(void* member_ptr) {

	if(member_ptr == &layout_size) {
		layout_size = cgv::math::clamp(layout_size, 10, 2000);
		overlay::set_size(ivec2(layout_size));
	}

	update_member(member_ptr);
	post_redraw();
}

bool navigator::init(context& ctx) {
	
	// get a bold font face to use for the cursor
	auto font = cgv::media::font::find_font("Arial");
	if(!font.empty()) {
		cursor_font_face = font->get_font_face(cgv::media::font::FFA_BOLD);
	}

	fbc.add_attachment("depth", "[D]");
	fbc.add_attachment("color", "flt32[R,G,B,A]", TF_LINEAR);
	fbc.set_size(2 * get_rectangle().size);
	
	bool success = true;

	success &= fbc.ensure(ctx);
	
	success &= arrow_data.init(ctx);
	success &= box_data.init(ctx);
	success &= box_wire_data.init(ctx);
	success &= rectangle_data.init(ctx);
	success &= sphere_data.init(ctx);

	success &= box_renderer.init(ctx);

	blit_canvas.register_shader("rectangle", cgv::g2d::shaders::rectangle);
	success &= blit_canvas.init(ctx);

	if(success) {
		box_data.add_position(vec3(0.0f));
		box_wire_data.add_position(vec3(0.0f));
		
		sphere_data.add_position(vec3(0.0f));

		const float length = 0.5f;

		// x - red
		arrow_data.add(vec3(0.0f), rgb(0.85f, 0.0f, 0.0f), vec3(length, 0.0f, 0.0f));
		// y - green
		arrow_data.add(vec3(0.0f), rgb(0.0f, 0.75f, 0.0f), vec3(0.0f, length, 0.0f));
		// z - blue
		arrow_data.add(vec3(0.0f), rgb(0.0f, 0.05f, 0.95f), vec3(0.0f, 0.0f, length));
		
		blit_style.fill_color = rgba(1.0f);
		blit_style.use_texture = true;
		blit_style.use_blending = true;
		blit_style.feather_width = 0.0f;

		auto& blit_prog = blit_canvas.enable_shader(ctx, "rectangle");
		blit_style.apply(ctx, blit_prog);
		blit_canvas.disable_current_shader(ctx);
	}

	return success;
}

void navigator::init_frame(context& ctx) {

	if(!view_ptr)
		view_ptr = find_view_as_node();

	if(ensure_layout(ctx)) {
		fbc.set_size(2 * get_rectangle().size);
		fbc.ensure(ctx);

		blit_canvas.set_resolution(ctx, get_viewport_size());
	}
}

void navigator::finish_draw(context& ctx) {

	fbc.enable(ctx);

	ctx.push_bg_color();
	ctx.set_bg_color({ 0.0f });
	ctx.clear_background(true, true);
	ctx.pop_bg_color();

	ctx.push_projection_matrix();
	ctx.set_projection_matrix(get_projection_matrix());

	ctx.push_modelview_matrix();
	ctx.set_modelview_matrix(get_view_matrix(ctx) * get_model_matrix(ctx));

	ctx.push_depth_test_state();
	ctx.disable_depth_test();

	ctx.push_blend_state();
	ctx.enable_blending();
	ctx.set_blend_func_separate(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA, BF_ONE, BF_ONE_MINUS_SRC_ALPHA);
	
	if(show_box)
		box_data.render(ctx, box_renderer);

	if(show_wireframe)
		box_wire_data.render(ctx);

	sphere_data.render(ctx);
	arrow_data.render(ctx);

	if(hit_axis != 0) {
		int axis_idx = abs(hit_axis) - 1;
		vec3 position(0.0f);
		position[axis_idx] = hit_axis < 0.0f ? -0.5f : 0.5f;

		const int mapping[3] = {1, 0, 2};

		vec3 rotation_axis(0.0f);
		rotation_axis[mapping[axis_idx]] = hit_axis < 0.0f ? -1.0f : 1.0f;

		std::vector<quat> rotations;
		quat rotation(rotation_axis, cgv::math::deg2rad(90.0f));

		rectangle_data.clear();
		rectangle_data.add_position(position);
		rectangle_data.add_rotation(rotation);

		auto& rectangle_renderer = ref_rectangle_renderer(ctx);
		rectangle_renderer.set_extent(ctx, vec2(0.9f));
		rectangle_data.render(ctx, rectangle_renderer);
	}

	ctx.pop_modelview_matrix();
	ctx.pop_projection_matrix();

	fbc.disable(ctx);

	ctx.set_blend_func(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);

	// draw frame buffer texture to screen
	auto& blit_prog = blit_canvas.enable_shader(ctx, "rectangle");

	fbc.enable_attachment(ctx, "color", 0);
	blit_canvas.draw_shape(ctx, get_rectangle());
	fbc.disable_attachment(ctx, "color");

	blit_canvas.disable_current_shader(ctx);

	ctx.pop_blend_state();
	ctx.pop_depth_test_state();
}

void navigator::set_size(int size) {

	layout_size = size;
	on_set(&layout_size);
}

void navigator::create_gui_impl() {

	add_member_control(this, "Size", layout_size, "value_slider", "min=50;max=300;step=1;ticks=true");
	add_member_control(this, "Show Box", show_box, "check");
	add_member_control(this, "Show Wireframe", show_wireframe, "check");
	add_member_control(this, "Use Perspective", use_perspective, "check");
}

mat4 navigator::get_model_matrix(context& ctx) {

	mat4 MV = ctx.get_modelview_matrix();

	// remove translation
	MV(0, 3) = 0.0f;
	MV(1, 3) = 0.0f;
	MV(2, 3) = 0.0f;

	return MV;
}

mat4 navigator::get_view_matrix(context& ctx) {

	return cgv::math::look_at4(navigator_eye_pos, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
}

mat4 navigator::get_projection_matrix() {

	vec2 size = static_cast<vec2>(get_rectangle().size);
	float aspect = size.x() / size.y();
	
	if(use_perspective)
		return cgv::math::perspective4(45.0f, aspect, 0.1f, 10.0f);
	else
		return cgv::math::ortho4(-1.0f, 1.0f, -1.0f, 1.0f, 0.01f, 5.0f);
}

bool navigator::intersect_box(const vec3 &origin, const vec3& direction, float& t) const {

	vec3 min(-0.5f);
	vec3 max(+0.5f);

	float t_min, t_max;

	vec3 inv_dir = vec3(1.0f) / direction;

	float t1 = (min.x() - origin.x())*inv_dir.x();
	float t2 = (max.x() - origin.x())*inv_dir.x();

	t_min = std::min(t1, t2);
	t_max = std::max(t1, t2);

	t1 = (min.y() - origin.y())*inv_dir.y();
	t2 = (max.y() - origin.y())*inv_dir.y();

	t_min = std::max(t_min, std::min(t1, t2));
	t_max = std::min(t_max, std::max(t1, t2));

	t1 = (min.z() - origin.z())*inv_dir.z();
	t2 = (max.z() - origin.z())*inv_dir.z();

	t_min = std::max(t_min, std::min(t1, t2));
	t_max = std::min(t_max, std::max(t1, t2));

	t = t_min;
	return t_max > std::max(t_min, 0.0f);
}

}
}
