#include "simple_object.h"
#include <cgv/math/proximity.h>
#include <cgv/math/intersection.h>

cgv::render::shader_program simple_object::prog;

#define USE_SCALABLE
//#define DEBUG_INTERSECTION

simple_object::rgb simple_object::get_modified_color(const rgb& color) const
{
	rgb mod_col(color);
	switch (state) {
	case state_enum::grabbed:
		mod_col[1] = std::min(1.0f, mod_col[0] + 0.2f);
	case state_enum::close:
		mod_col[0] = std::min(1.0f, mod_col[0] + 0.2f);
		break;
	case state_enum::triggered:
		mod_col[1] = std::min(1.0f, mod_col[0] + 0.2f);
	case state_enum::pointed:
		switch (pointed) {
		case pointed_type::translate:
			mod_col[0] = std::min(1.0f, mod_col[0] + 0.2f);
			break;
		case pointed_type::rotate:
			mod_col[1] = std::min(1.0f, mod_col[1] + 0.2f);
			break;
		case pointed_type::scale:
			mod_col[2] = std::min(1.0f, mod_col[2] + 0.2f);
			break;
		}
		break;
	}
	return mod_col;
}
simple_object::simple_object(const std::string& _name, const vec3& _position, const rgb& _color, const vec3& _extent, const quat& _rotation)
	: cgv::base::node(_name), 
	  cgv::nui::concatenating_transforming<
	    cgv::nui::default_translatable, 
	    cgv::nui::quaternion_rotatable,
	    cgv::nui::non_uniformly_scalable>(
		_position, _rotation,
#ifdef USE_SCALABLE
			_extent), extent(vec3(1.0f)),

#else
			vec3(1.0f)), extent(_extent), 
#endif
	color(_color)
	//, position(_position), rotation(_rotation)
{
	debug_point = position + 0.5f*extent;
	brs.rounding = true;
	brs.default_radius = 0.02f;
	srs.radius = 0.01f;
}
std::string simple_object::get_type_name() const
{
	return "simple_object";
}
void simple_object::on_set(void* member_ptr)
{
	update_member(member_ptr);
	post_redraw();
}
bool simple_object::focus_change(cgv::nui::focus_change_action action, cgv::nui::refocus_action rfa, const cgv::nui::focus_demand& demand, const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info)
{
	switch (action) {
	case cgv::nui::focus_change_action::attach:
		if (state == state_enum::idle) {
			// set state based on dispatch mode
			state = dis_info.mode == cgv::nui::dispatch_mode::pointing ? state_enum::pointed : state_enum::close;
			on_set(&state);
			// store hid to filter handled events
			hid_id = dis_info.hid_id;
			return true;
		}
		// if focus is given to other hid, refuse attachment to new hid
		return false;
	case cgv::nui::focus_change_action::detach:
		// check whether detach corresponds to stored hid
		if (state != state_enum::idle && hid_id == dis_info.hid_id) {
			state = state_enum::idle;
			on_set(&state);
			return true;
		}
		return false;
	case cgv::nui::focus_change_action::index_change:
		// nothing to be done because with do not use indices
		break;
	}
	return true;
}
void simple_object::stream_help(std::ostream& os)
{
	os << "simple_object: grab and point at it" << std::endl;
}
bool simple_object::handle(const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info, cgv::nui::focus_request& request)
{
	// ignore all events in idle mode
	if (state == state_enum::idle)
		return false;
	// ignore events from other hids
	if (!(dis_info.hid_id == hid_id))
		return false;
	bool pressed;
	// hid independent check if grabbing is activated or deactivated
	if (is_grab_change(e, pressed)) {
		if (pressed) {
			state = state_enum::grabbed;
			on_set(&state);
			drag_begin(request, false, original_config);
		}
		else {
			drag_end(request, original_config);
			state = state_enum::close;
			on_set(&state);
		}
		return true;
	}
	// check if event is for grabbing
	if (is_grabbing(e, dis_info)) {
		const auto& prox_info = get_proximity_info(dis_info);
		if (state == state_enum::close) {
			debug_point = prox_info.hit_point;
			query_point_at_grab = prox_info.query_point;
			position_at_grab = position;
		}
		else if (state == state_enum::grabbed) {
			debug_point = prox_info.hit_point;
			position = position_at_grab + prox_info.query_point - query_point_at_grab;
		}
		post_redraw();
		return true;
	}
	// hid independent check if object is triggered during pointing
	if (is_trigger_change(e, pressed)) {
		if (pressed) {
			state = state_enum::triggered;
			on_set(&state);
			drag_begin(request, true, original_config);
		}
		else {
			drag_end(request, original_config);
			state = state_enum::pointed;
			on_set(&state);
		}
		return true;
	}
	// check if event is for pointing
	if (is_pointing(e, dis_info)) {
		const auto& inter_info = get_intersection_info(dis_info);
		if (state == state_enum::pointed) {
			debug_point = inter_info.hit_point;
			int i = max_index(abs(debug_point));
			float r = sqrt(debug_point[(i + 1) % 3] * debug_point[(i + 1) % 3] + debug_point[(i + 2) % 3] * debug_point[(i + 2) % 3]);
			pointed = r < 0.2f ? pointed_type::translate : (r < 0.55f ? pointed_type::rotate : pointed_type::scale);
			switch (pointed) {
			case pointed_type::translate:
				hit_point_at_trigger = transform_point(inter_info.hit_point);
				position_at_trigger = position;
				break;
			case pointed_type::rotate:
				hit_point_at_trigger = transform_point(inter_info.hit_point) - position;
				quaternion_at_trigger = quaternion;
				break;
			case pointed_type::scale:
				hit_point_at_trigger = inter_info.hit_point;
				hit_point_at_trigger *= scale;
				scale_at_trigger = scale;
				break;
			}
		}
		else if (state == state_enum::triggered) {
			// if we still have an intersection point, use as debug point
			if (inter_info.ray_param != std::numeric_limits<float>::max())
				debug_point = inter_info.hit_point;
			switch (pointed) {
			case pointed_type::translate: {
				vec3 line_projection = cgv::math::closest_point_on_line_to_point(
					transform_point(inter_info.ray_origin),
					transform_vector(inter_info.ray_direction),
					hit_point_at_trigger);
				vec3 translation = line_projection - hit_point_at_trigger;
				// compute translated position
				vec3 new_position = position_at_trigger + translation;
				position = new_position;
				break;
			}
			case pointed_type::rotate: {
				vec3 line_projection = cgv::math::closest_point_on_line_to_point(
					transform_point(inter_info.ray_origin) - position,
					transform_vector(inter_info.ray_direction),
					hit_point_at_trigger);
				vec3 axis = cross(hit_point_at_trigger, line_projection);
				float angle = atan2(axis.normalize(), dot(hit_point_at_trigger, line_projection));
				quat rotation(axis, angle);
				quaternion = rotation * quaternion_at_trigger;
				break;
			}
			case pointed_type::scale: {
				vec3 ro = inter_info.ray_origin, rd = inter_info.ray_direction;
				ro *= scale; rd *= scale;
				vec3 line_projection = cgv::math::closest_point_on_line_to_point(ro, rd, hit_point_at_trigger);
				vec3 new_scale = scale;
				for (int i=0; i<3; ++i)
					new_scale[i] = std::min(10.0f, std::max(0.01f, line_projection[i] / hit_point_at_trigger[i]));
				scale = new_scale * scale_at_trigger;
				break;
			}
			}
		}
		post_redraw();
		return true;
	}
	return false;
}
bool simple_object::compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx)
{
	prj_point = point;
	for (int i = 0; i < 3; ++i)
		prj_point[i] = std::max(-0.5f * extent[i], std::min(0.5f * extent[i], prj_point[i]));
	return true;
}
bool simple_object::compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx)
{
	vec3 n;
	vec2 res;
	int nr_intersect = cgv::math::ray_box_intersection(cgv::math::ray<float, 3>(ray_start, ray_direction), 0.5f*extent, res, &n);
#ifdef DEBUG_INTERSECTION
	std::cout << get_name() << " [" << extent << "] : ray {" << ray_start << "|" << ray_direction << "}";
	for (int i = 0; i < nr_intersect; ++i)
		std::cout << " " << res[0];
#endif
	if (nr_intersect == 0) {
#ifdef DEBUG_INTERSECTION
		std::cout << " no intersection" << std::endl;
#endif
		return false;
	}
	if (res[0] < 0) {
		if (res[1] < 0)
			return false;
		hit_param = res[1];
	}
	else {
		hit_param = res[0];
	}
	hit_normal = n;
#ifdef DEBUG_INTERSECTION
	std::cout << " interset at " << hit_param << std::endl;
#endif
	return true;
}
bool simple_object::init(cgv::render::context& ctx)
{
	cgv::render::ref_sphere_renderer(ctx, 1);
	auto& br = cgv::render::ref_box_renderer(ctx, 1);
	if (prog.is_linked())
		return true;
	return br.build_program(ctx, prog, brs);
}
void simple_object::clear(cgv::render::context& ctx)
{
	cgv::render::ref_box_renderer(ctx, -1);
	cgv::render::ref_sphere_renderer(ctx, -1);
}
void simple_object::draw(cgv::render::context& ctx)
{
	// show box
	auto& br = cgv::render::ref_box_renderer(ctx);
	br.set_render_style(brs);
	if (brs.rounding)
		br.set_prog(prog);
	br.set_position(ctx, position);
	br.set_color_array(ctx, &color, 1);
	br.set_secondary_color(ctx, get_modified_color(color));
#ifdef USE_SCALABLE
	br.set_extent(ctx, scale);
#else
	br.set_extent(ctx, extent);
#endif
	br.set_rotation_array(ctx, &quaternion, 1);
	br.render(ctx, 0, 1);

	// show points
	ctx.push_modelview_matrix();
	ctx.mul_modelview_matrix(get_model_transform());
	auto& sr = cgv::render::ref_sphere_renderer(ctx);
	sr.set_render_style(srs);
	sr.set_position(ctx, debug_point);
	sr.set_color_array(ctx, &color, 1);
	sr.render(ctx, 0, 1);
	if (state == state_enum::grabbed) {
		sr.set_position(ctx, query_point_at_grab);
		sr.set_color(ctx, rgb(0.5f, 0.5f, 0.5f));
		sr.render(ctx, 0, 1);
	}
	if (state == state_enum::triggered) {
		vec3 hpat = inverse_transform_point(hit_point_at_trigger);
		sr.set_position(ctx, hpat);
		sr.set_color(ctx, rgb(0.3f, 0.3f, 0.3f));
		sr.render(ctx, 0, 1);
	}
	ctx.pop_modelview_matrix();
}
void simple_object::create_gui()
{
	add_decorator(get_name(), "heading", "level=2");
	add_member_control(this, "color", color);
#ifdef USE_SCALABLE
	add_member_control(this, "width",  scale[0], "value_slider", "min=0.01;max=1;log=true");
	add_member_control(this, "height", scale[1], "value_slider", "min=0.01;max=1;log=true");
	add_member_control(this, "depth",  scale[2], "value_slider", "min=0.01;max=1;log=true");
#else
	add_member_control(this, "width",  extent[0], "value_slider", "min=0.01;max=1;log=true");
	add_member_control(this, "height", extent[1], "value_slider", "min=0.01;max=1;log=true");
	add_member_control(this, "depth",  extent[2], "value_slider", "min=0.01;max=1;log=true");
#endif
	add_gui("rotation", quaternion, "direction", "options='min=-1;max=1;ticks=true'");
	if (begin_tree_node("style", brs)) {
		align("\a");
		add_gui("brs", brs);
		align("\b");
		end_tree_node(brs);
	}
}
