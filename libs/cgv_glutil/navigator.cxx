#include "navigator.h"

#include <cgv/defines/quote.h>
#include <cgv/gui/dialog.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/dialog.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/math/ftransform.h>
//#include <cgv/utils/advanced_scan.h>
//#include <cgv/utils/file.h>
#include <cgv_gl/gl/gl.h>

namespace cgv {
namespace glutil {

navigator::navigator() {
	
	set_name("Navigator");

	view_ptr = nullptr;
	navigator_eye_pos = vec3(0.0f, 0.0f, 4.0f);

	layout.padding = 8;
	layout.total_height = 200;
	layout.color_scale_height = 30;

	set_overlay_alignment(AO_CENTER, AO_CENTER);
	set_overlay_stretch(SO_NONE);
	set_overlay_margin(ivec2(0));
	set_overlay_size(ivec2(300));
	
	fbc.add_attachment("color", "flt32[R,G,B,A]");
	fbc.set_size(get_overlay_size());

	//canvas.register_shader("rectangle", "rect2d.glpr");
	//canvas.register_shader("circle", "circle2d.glpr");
	//canvas.register_shader("histogram", "hist2d.glpr");
	//canvas.register_shader("background", "bg2d.glpr");

	//overlay_canvas.register_shader("rectangle", "rect2d.glpr");

	//hit_box = false;
	hit_axis = 0;

	box_data = cgv::glutil::box_render_data<>(true);
	sphere_data = cgv::glutil::sphere_render_data<>(true);
	cone_data = cgv::glutil::cone_render_data<>(true);

	box_style.default_extent = vec3(1.0f);
	box_style.map_color_to_material = CM_COLOR_AND_OPACITY;
	box_style.surface_color = rgb(0.5f);
	box_style.surface_opacity = 0.5f;

	sphere_style.radius = 0.02f;
	sphere_style.surface_color = rgb(0.5f);

	cone_style.radius = 0.01f;

	rectangle_style.illumination_mode = IM_OFF;
	rectangle_style.map_color_to_material = CM_COLOR_AND_OPACITY;
	rectangle_style.surface_color = rgb(0.25f, 0.5f, 1.0f);
	rectangle_style.surface_opacity = 0.75f;
	rectangle_style.pixel_blend = 0.0f;
	rectangle_style.percentual_border_width = 0.1f;
	rectangle_style.border_color = rgba(0.15f, 0.4f, 0.9f, 0.75f);

	mouse_is_on_overlay = false;
	show_cursor = false;
	cursor_pos = ivec2(-100);
	cursor_drawtext = "";
}

void navigator::clear(cgv::render::context& ctx) {

	//canvas.destruct(ctx);
	//overlay_canvas.destruct(ctx);
	fbc.clear(ctx);

	ref_box_renderer(ctx, -1);
	ref_sphere_renderer(ctx, -1);
	ref_cone_renderer(ctx, -1);
	ref_rectangle_renderer(ctx, -1);
}

bool navigator::self_reflect(cgv::reflect::reflection_handler& _rh) {

	return true;// _rh.reflect_member("file_name", file_name);
}

bool navigator::handle_event(cgv::gui::event& e) {

	// return true if the event gets handled and stopped here or false if you want to pass it to the next plugin
	unsigned et = e.get_kind();
	unsigned char modifiers = e.get_modifiers();

	if(!show)
		return false;

	if(et == cgv::gui::EID_KEY) {
		cgv::gui::key_event& ke = (cgv::gui::key_event&) e;

		if(ke.get_action() == cgv::gui::KA_PRESS) {
			switch(ke.get_key()) {
			case cgv::gui::KEY_Left_Ctrl:
				show_cursor = true;
				cursor_drawtext = "+";
				post_redraw();
				break;
			case cgv::gui::KEY_Left_Alt:
				show_cursor = true;
				cursor_drawtext = "-";
				post_redraw();
				break;
			}
		} else if(ke.get_action() == cgv::gui::KA_RELEASE) {
			switch(ke.get_key()) {
			case cgv::gui::KEY_Left_Ctrl:
				show_cursor = false;
				post_redraw();
				break;
			case cgv::gui::KEY_Left_Alt:
				show_cursor = false;
				post_redraw();
				break;
			}
		}
	} else if(et == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = (cgv::gui::mouse_event&) e;
		cgv::gui::MouseAction ma = me.get_action();

		int last_hit_axis = hit_axis;
		hit_axis = 0;
		//bool last_hit_box = hit_box;
		//hit_box = false;

		if(get_context()) {
			cgv::render::context& ctx = *get_context();

			ivec2 mpos(static_cast<int>(me.get_x()), static_cast<int>(me.get_y()));
			mpos = get_transformed_mouse_pos(mpos);
			vec2 window_coord = vec2(mpos) * vec2(2.0f) / last_viewport_size - vec2(1.0f);

			mat4 MVP = ctx.get_projection_matrix() * get_view_matrix(ctx);

			vec4 world_coord(window_coord.x(), window_coord.y(), 1.0f, 1.0f);
			world_coord = inv(MVP) * world_coord;
			world_coord /= world_coord.w();

			vec3 origin = navigator_eye_pos;
			vec3 direction = normalize(vec3(world_coord) - origin);
			
			mat4 IM = inv(get_model_matrix(ctx));

			origin = vec3(IM * vec4(origin, 1.0f));
			direction = vec3(IM * vec4(direction, 0.0f));

			float t = std::numeric_limits<float>::max();
			if(intersect_box(origin, direction, t)) {
				vec3 hit_pos = origin + t * direction;

				unsigned mi = cgv::math::max_index(cgv::math::abs(hit_pos));
				//char cc[3] = { 'x', 'y', 'z' };
				//char v = hit_pos[mi] < 0.0f ? '-' : '+';

				//hit_box = true;
				hit_axis = hit_pos[mi] < 0.0f ? -(mi+1) : (mi+1);
				//hit_normal = vec3(0.0f);
				//hit_normal[mi] = hit_pos[mi] < 0.0f ? -1.0f : 1.0f;

				//std::cout << v << cc[mi] << std::endl;
			}
			//if(last_hit_box != hit_box || last_hit_axis != hit_axis)
			if(last_hit_axis != hit_axis)
				post_redraw();
		}

		switch(ma) {
			/*case MA_PRESS:
				if(me.get_button() == MB_LEFT_BUTTON && me.get_modifiers() == 0) {
					check_for_click = me.get_time();
					return true;
				}
				if(((me.get_button() == MB_LEFT_BUTTON) &&
					((me.get_modifiers() == 0) || (me.get_modifiers() == EM_SHIFT))) ||
					((me.get_button() == MB_RIGHT_BUTTON) && (me.get_modifiers() == 0)) ||
					((me.get_button() == MB_MIDDLE_BUTTON) && (me.get_modifiers() == 0)))
					return true;
				break;*/
		case cgv::gui::MA_RELEASE:
			//if(check_for_click != -1) {
			//	double dt = me.get_time() - check_for_click;
			//	if(dt < 0.2) {

		/*if(get_context() && view_ptr) {
			cgv::render::context& ctx = *get_context();

			ivec2 mpos(static_cast<int>(me.get_x()), static_cast<int>(me.get_y()));
			mpos = get_transformed_mouse_pos(mpos);
			vec2 window_coord = vec2(mpos) * vec2(2.0f)/last_viewport_size - vec2(1.0f);
			
			mat4 MVP = ctx.get_projection_matrix() * ctx.get_modelview_matrix();

			vec4 world_coord(window_coord.x(), window_coord.y(), 1.0f, 1.0f);
			world_coord = inv(MVP) * world_coord;
			world_coord /= world_coord.w();

			vec3 direction = normalize(vec3(world_coord) - view_ptr->get_eye());
			std::cout << direction << std::endl;
		}*/



		/*{
			// RAYCASTING
			// Transform fragment coordinates from window coordinates to view coordinates.
			coord = gl_FragCoord
				* vec4(view.viewAttributes.z, view.viewAttributes.w, 2.0, 0.0)
				+ vec4(-1.0, -1.0, -1.0, 1.0);

			// Transform fragment coordinates from view coordinates to object coordinates-
			coord = ubo.modelViewProjectionMatrixInverse * coord;
			coord /= coord.w;
			coord -= in_pos; // and to glyph space

			// Calculate the viewing ray.
			ray = normalize(coord.xyz - camPos.xyz);
		}*/


					if(get_context() && view_ptr) {
						cgv::render::context& ctx = *get_context();
						dvec3 p;
						double z = view_ptr->get_z_and_unproject(ctx, me.get_x(), me.get_y(), p);
						if(z > 0 && z < 1) {
							
							dvec3 e = view_ptr->get_eye();
							double l_old = (e - view_ptr->get_focus()).length();
							double l_new = dot(p - e, view_ptr->get_view_dir());

							//cgv::gui::animate_with_geometric_blend(view_ptr->ref_y_extent_at_focus(), view_ptr->get_y_extent_at_focus() * l_new / l_old, 0.5)->set_base_ptr(this);
							//
							//cgv::gui::animate_with_linear_blend(view_ptr->ref_focus(), p, 0.5)->configure(cgv::gui::APM_SIN_SQUARED, this);

							std::cout << p << std::endl;

							//update_vec_member(view::focus);
							post_redraw();
							return true;
						}
					}
				//}
				//check_for_click = -1;
			//}
			//if((me.get_button() == MB_LEFT_BUTTON && (me.get_modifiers() == 0 || me.get_modifiers() == EM_SHIFT)) ||
			//	me.get_button() == MB_RIGHT_BUTTON && me.get_modifiers() == 0)
			//	return true;
			break;
		}

		/*
		if (get_context()) {
						cgv::render::context& ctx = *get_context();
						dvec3 p;
						double z = get_z_and_unproject(ctx, me.get_x(), me.get_y(), p);
						if (z > 0 && z < 1) {
							if (y_view_angle > 0.1) {
								dvec3 e = view_ptr->get_eye();
								double l_old = (e - view_ptr->get_focus()).length();
								double l_new = dot(p - e, view_ptr->get_view_dir());

								cgv::gui::animate_with_geometric_blend(view_ptr->ref_y_extent_at_focus(), view_ptr->get_y_extent_at_focus() * l_new / l_old, 0.5)->set_base_ptr(this);
							}
							cgv::gui::animate_with_linear_blend(view_ptr->ref_focus(), p, 0.5)->configure(cgv::gui::APM_SIN_SQUARED, this);

							update_vec_member(view::focus);
							post_redraw();
							return true;
						}
					}
		*/




		/*switch(ma) {
		case cgv::gui::MA_ENTER:
			mouse_is_on_overlay = true;
			break;
		case cgv::gui::MA_LEAVE:
			mouse_is_on_overlay = false;
			post_redraw();
			break;
		case cgv::gui::MA_MOVE:
		case cgv::gui::MA_DRAG:
			cursor_pos = ivec2(me.get_x(), me.get_y());
			if(show_cursor)
				post_redraw();
			break;
		}

		if(me.get_button_state() & cgv::gui::MB_LEFT_BUTTON) {
			if(ma == cgv::gui::MA_PRESS && modifiers > 0) {
				ivec2 mpos = get_local_mouse_pos(ivec2(me.get_x(), me.get_y()));

				switch(modifiers) {
				case cgv::gui::EM_CTRL:
					//if(!get_hit_point(mpos))
					//	add_point(mpos);
					break;
				case cgv::gui::EM_ALT:
				{
					//point* hit_point = get_hit_point(mpos);
					//if(hit_point)
					//	remove_point(hit_point);
				}
				break;
				}
			}
		}

		//return tfc.points.handle(e, last_viewport_size, container);*/

		return false;
	} else {
		return false;
	}
}

void navigator::on_set(void* member_ptr) {

	if(member_ptr == &layout.total_height) {
		ivec2 size = get_overlay_size();
		size.y() = layout.total_height;
		set_overlay_size(size);
	}

	update_member(member_ptr);
	post_redraw();
}

bool navigator::init(cgv::render::context& ctx) {
	
	// get a bold font face to use for the cursor
	auto font = cgv::media::font::find_font("Arial");
	if(!font.empty()) {
		cursor_font_face = font->get_font_face(cgv::media::font::FFA_BOLD);
	}

	bool success = true;

	success &= fbc.ensure(ctx);
	//success &= canvas.init(ctx);
	//success &= overlay_canvas.init(ctx);
	success &= box_data.init(ctx);
	success &= sphere_data.init(ctx);
	success &= cone_data.init(ctx);

	ref_box_renderer(ctx, 1);
	ref_sphere_renderer(ctx, 1);
	ref_cone_renderer(ctx, 1);
	ref_rectangle_renderer(ctx, 1);

	if(success) {
		box_data.add(vec3(0.0f));

		sphere_data.add(vec3(0.0f));

		cone_data.add(vec3(0.0f), vec3(0.35f, 0.0f, 0.0f));
		cone_data.add(rgb(1.0f, 0.0f, 0.0f));
		cone_data.add(vec3(0.0f), vec3(0.0f, 0.35f, 0.0f));
		cone_data.add(rgb(0.0f, 1.0f, 0.0f));
		cone_data.add(vec3(0.0f), vec3(0.0f, 0.0f, 0.35f));
		cone_data.add(rgb(0.0f, 0.0f, 1.0f));
	}

	return success;
}

void navigator::init_frame(cgv::render::context& ctx) {

	if(!view_ptr)
		view_ptr = find_view_as_node();

	if(ensure_overlay_layout(ctx)) {
		ivec2 container_size = get_overlay_size();
		layout.update(container_size);

		fbc.set_size(container_size);
		fbc.ensure(ctx);

		//canvas.set_resolution(ctx, container_size);
		//overlay_canvas.set_resolution(ctx, get_viewport_size());

		//auto& bg_prog = canvas.enable_shader(ctx, "background");
		//float width_factor = static_cast<float>(layout.editor_rect.size().x()) / static_cast<float>(layout.editor_rect.size().y());
		//bg_style.texcoord_scaling = vec2(5.0f * width_factor, 5.0f);
		//bg_style.apply(ctx, bg_prog);
		//canvas.disable_current_shader(ctx);
	}
}

void navigator::finish_draw(cgv::render::context& ctx) {

	if(!show)
		return;

	//fbc.enable(ctx);
	
	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//glClear(GL_COLOR_BUFFER_BIT);
	
	ctx.push_modelview_matrix();
	ctx.set_modelview_matrix(get_view_matrix(ctx) * get_model_matrix(ctx));

	sphere_data.render(ctx, ref_sphere_renderer(ctx), sphere_style);
	cone_data.render(ctx, ref_cone_renderer(ctx), cone_style);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	box_data.render(ctx, ref_box_renderer(ctx), box_style);

	if(hit_axis != 0) {
		int axis_idx = abs(hit_axis) - 1;
		vec3 position(0.0f);
		position[axis_idx] = hit_axis < 0.0f ? -0.5f : 0.5f;

		std::vector<vec3> positions;
		positions.push_back(position);

		const int mapping[3] = {1, 0, 2};

		vec3 rotation_axis(0.0f);
		rotation_axis[mapping[axis_idx]] = hit_axis < 0.0f ? -1.0f : 1.0f;

		std::vector<quat> rotations;
		quat q(rotation_axis, cgv::math::deg2rad(90.0f));
		rotations.push_back(q);

		auto& rectangle_renderer = ref_rectangle_renderer(ctx);
		rectangle_renderer.set_render_style(rectangle_style);
		rectangle_renderer.set_extent(ctx, vec2(1.0f));
		rectangle_renderer.set_position_array(ctx, positions);
		rectangle_renderer.set_rotation_array(ctx, rotations);
		rectangle_renderer.render(ctx, 0, 1);
	}

	ctx.pop_modelview_matrix();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void navigator::create_gui() {

	add_decorator("Transfer Function Editor", "heading", "level=2");

	create_overlay_gui();
}

void navigator::create_gui(cgv::gui::provider& p) {

	p.add_member_control(this, "Show", show, "check");
}

navigator::mat4 navigator::get_model_matrix(context& ctx) {

	mat4 MV = ctx.get_modelview_matrix();

	// remove translation
	MV(0, 3) = 0.0f;
	MV(1, 3) = 0.0f;
	MV(2, 3) = 0.0f;

	return MV;
}

navigator::mat4 navigator::get_view_matrix(context& ctx) {

	return cgv::math::look_at4(navigator_eye_pos, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
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
