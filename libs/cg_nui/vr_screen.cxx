#include "vr_screen.h"
#include <cgv_gl/sphere_renderer.h>
#include <cgv/math/ftransform.h>
#include <cgv/math/pose.h>
#include <cgv/os/mouse_ctrl.h>
#include <cg_vr/vr_events.h>
#include <3rd/screen_capture_lite/include/ScreenCapture.h>

namespace cgv {
	namespace nui {

/*
void enum_monitors()
{
	// will return all attached monitors
	auto M = SL::Screen_Capture::GetMonitors();
	for (auto m : M) {
		std::cout << m.Name << "(" << m.Width << "x" << m.Height << ")@" << m.OffsetX << "," << m.OffsetY << std::endl;
	}
}

void enum_windows()
{
	auto W = SL::Screen_Capture::GetWindows();
	for (auto w : W) {
		if (std::string(w.Name).empty())
			continue;
		if (w.Size.x * w.Size.y == 0)
			continue;
		std::cout << w.Handle << ":" << w.Position.x << "," << w.Position.y << " - " << w.Size.x << "x" << w.Size.y << ": " << w.Name << std::endl;
	}
}
*/
void vr_screen::screen_capture_callback(const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor)
{
	int i = SL::Screen_Capture::Index(monitor);
	//std::cout << "received image: " << SL::Screen_Capture::Width(img) << "x" << SL::Screen_Capture::Height(img) 
	//	<< " from monitor " << i 
	//	<< "(" << SL::Screen_Capture::Width(monitor) << "x" << SL::Screen_Capture::Height(monitor) << ")" << std::endl;

	// copy image to data view and mark dirts
	SL::Screen_Capture::Extract(img, screen_images[i].get_ptr<unsigned char>(), screen_image_dfs[i].get_nr_bytes());
	screen_image_dirty[i] = true;
}

vr_screen::vr_screen(const std::string& name) : cgv::base::node(name)
{
	enabled = true;
	screen_style.percentual_border_width = 0.04f;
	screen_style.illumination_mode = cgv::render::IM_OFF;
	// allocate a data view for each monitor
	const auto& M = SL::Screen_Capture::GetMonitors();
	screen_images.resize(M.size());
	screen_image_dfs.resize(M.size());
	screen_image_dirty.resize(M.size());
	for (size_t i = 0; i < M.size(); ++i) {
		screen_image_dfs[i].set_component_format(cgv::data::component_format(cgv::type::info::TI_UINT8, cgv::data::CF_BGRA));
		screen_image_dfs[i].set_width(SL::Screen_Capture::Width(M[i]));
		screen_image_dfs[i].set_height(SL::Screen_Capture::Height(M[i]));
		new (&screen_images[i]) cgv::data::data_view(&screen_image_dfs[i]);
	}
	screen_aspect = float(screen_image_dfs[0].get_width()) / screen_image_dfs[0].get_height();

	// Setup Screen Capture for all monitors
	auto framegrabber = SL::Screen_Capture::CreateCaptureConfiguration([]() { return SL::Screen_Capture::GetMonitors(); });
	framegrabber->onNewFrame([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
		this->screen_capture_callback(img, monitor);
	});
	//onFrameChanged([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor);
	//onMouseChanged([&](const SL::Screen_Capture::Image* img, const SL::Screen_Capture::MousePoint& mousepoint);
	screen_capture_manager = framegrabber->start_capturing();
	screen_capture_manager->setFrameChangeInterval(std::chrono::milliseconds(100));
	screen_capture_manager->setMouseChangeInterval(std::chrono::milliseconds(100));
	screen_capture_manager->pause();
	screen_capture = false;
	mouse_button_pressed[0] = mouse_button_pressed[1] = mouse_button_pressed[2] = false;
	initial_placement = true;
	show_screen = false;
	hide();
}

bool vr_screen::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return
		rh.reflect_member("enabled", enabled) &&
		rh.reflect_member("scale", screen_scale) &&
		rh.reflect_member("show", show_screen) &&
		rh.reflect_member("style", screen_style);
}

void vr_screen::on_set(void* member_ptr)
{
	if (member_ptr == &screen_distance || member_ptr == &screen_x_offset || member_ptr == &screen_scale)
		place_screen();

	if (member_ptr == &screen_capture) {
		if (!screen_capture != screen_capture_manager->isPaused()) {
			if (screen_capture)
				screen_capture_manager->resume();
			else
				screen_capture_manager->pause();
		}
	}
	if (member_ptr == &show_screen) {
		if (show_screen != is_visible())
			if (show_screen)
				show();
			else
				hide();
	}

	update_member(member_ptr);
	post_redraw();
}

/// ask focusable what it wants to grab focus based on given event - in case of yes, the focus_demand should be filled
bool vr_screen::wants_to_grab_focus(const cgv::gui::event& e, const cgv::nui::hid_identifier& hid_id, cgv::nui::focus_demand& demand)
{
	// only grab focus when idle
	if (state != state_enum::idle || !enabled)
		return false;
	// ask to grab focus of vr kit if menu key of a vr control is pressed 
	if (e.get_kind() != cgv::gui::EID_KEY || e.get_flags() & ((cgv::gui::EF_VR) == 0))
		return false;
	const cgv::gui::vr_key_event& vrke = reinterpret_cast<const cgv::gui::vr_key_event&>(e);
	if (vrke.get_key() != vr::VR_MENU)
		return false;
	if (vrke.get_action() != cgv::gui::KA_PRESS)
		return false;
	if (vrke.get_state().hmd.status != vr::VRS_TRACKED)
		return false;
	if (vrke.get_state().controller[vrke.get_controller_index()].status != vr::VRS_TRACKED)
		return false;
	// grab focus of current kit
	demand.attach.level = cgv::nui::focus_level::kit;
	demand.attach.kit_id = { cgv::nui::get_kit_category(hid_id.category), hid_id.kit_ptr };
	// turn off spatial and structural dispatching and grabbing of focus
	demand.config.dispatch.spatial = false;
	demand.config.dispatch.focus_recursive = false;
	demand.config.dispatch.structural = false;
	demand.config.refocus.grab = false;
	demand.config.refocus.spatial = false;
	return true;
}
/// inform focusable that its focus changed (fst param tells whether receiving or loosing) together with focus request and causing event plus dispatch info
bool vr_screen::focus_change(cgv::nui::focus_change_action action, cgv::nui::refocus_action rfa, const cgv::nui::focus_demand& demand, const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info)
{
	// just keep track of focus type here and do anything else in the handle method
	if (action == cgv::nui::focus_change_action::attach) {
		if (rfa == cgv::nui::refocus_action::grab) {
			state = state_enum::place;
			update_member(&state);
			hid_id = dis_info.hid_id;
			return true;
		}
		else if (rfa == cgv::nui::refocus_action::intersection) {
			if (state == state_enum::idle) {
				state = state_enum::mouse;
				update_member(&state);
				hid_id = dis_info.hid_id;
				return true;
			}
		}
		return false;
	}
	else if (action == cgv::nui::focus_change_action::detach) {
		if (state == state_enum::mouse) {
			if (dis_info.hid_id == hid_id) {
				// ensure that mouse buttons pressed in emulation are also released again
				for (int i = 0; i < 3; ++i) {
					if (mouse_button_pressed[i]) {
						cgv::os::send_mouse_button_event(i, false);
						mouse_button_pressed[i] = false;
					}
				}
				state = state_enum::idle;
				update_member(&state);
			}
		}
		else {
			state = state_enum::idle;
			update_member(&state);
		}
	}
	return true;
}

/// stream help information to the given output stream
void vr_screen::stream_help(std::ostream& os)
{
}

void vr_screen::compute_mouse_xy(const vec3& p, int& X, int& Y) const
{
	vec3 d = p - screen_center; 
	screen_rotation.inverse_rotate(d);
	vec2 rd = ((const vec2&)d) / screen_extent + 0.5f;
	X = int(screen_texture.get_width() * rd.x());
	Y = screen_texture.get_height() - int(screen_texture.get_height() * rd.y());
}

void vr_screen::placement_trigger(const cgv::gui::vr_key_event& vrke, cgv::nui::focus_request& request)
{
	const auto& vr_state = vrke.get_state();
	int ci = vrke.get_controller_index();
	const auto& ctrl_state = vr_state.controller[ci];
	if (vrke.get_action() == cgv::gui::KA_PRESS) {
		start_placement_pose = cgv::math::pose_concat(
			cgv::math::pose_inverse(reinterpret_cast<const mat34&>(vr_state.hmd.pose[0])),
			reinterpret_cast<const mat34&>(ctrl_state.pose[0])
		);
		start_placement_time = vrke.get_time();
		if (initial_placement) {
			start_placement_time -= 0.2;
			place_screen(vr_state);
			initial_placement = false;
		}
		last_show_screen = show_screen;
		if (!is_visible()) {
			show_screen = true;
			update_member(&show_screen);
			show();
		}
		if (!screen_capture) {
			screen_capture = true;
			on_set(&screen_capture);
		}
	}
	else if (vrke.get_action() == cgv::gui::KA_RELEASE) {
		if (vrke.get_time() - start_placement_time < 0.2) {
			if (last_show_screen == show_screen) {
				if (show_screen) {
					show_screen = false;
					hide();
					if (screen_capture) {
						screen_capture = false;
						on_set(&screen_capture);
					}
				}
				else {
					show_screen = true;
					show();
					if (!screen_capture) {
						screen_capture = true;
						on_set(&screen_capture);
					}
				}
				update_member(&show_screen);
			}
		}
		else {
			if (ctrl_state.status == vr::VRS_TRACKED)
				place_screen(vr_state);
		}
		// in case of menu key release also release kit focus
		request.request = cgv::nui::focus_change_action::detach;
		request.demand.attach.level = cgv::nui::focus_level::kit;
	}
}

//! ask active focusable to handle event providing dispatch info
/*! return whether event was handled
	To request a focus change, fill the passed request struct and set the focus_change_request flag.*/
bool vr_screen::handle(const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info, cgv::nui::focus_request& request)
{
	if (state == state_enum::idle)
		return false;
	if (!(dis_info.hid_id == hid_id))
		return false;
	if ((e.get_flags() & cgv::gui::EF_VR) == 0)
		return false;
	if (state == state_enum::place) {
		if (e.get_kind() == cgv::gui::EID_KEY) {
			const auto& vrke = reinterpret_cast<const cgv::gui::vr_key_event&>(e);
			if (vrke.get_key() == vr::VR_MENU) {
				placement_trigger(vrke, request);
				return true;
			}
		}
		else if (e.get_kind() == cgv::gui::EID_POSE) {
			const auto& vrpe = reinterpret_cast<const cgv::gui::vr_pose_event&>(e);
			if (vrpe.get_time() - start_placement_time > 0.2) {
				place_screen(vrpe.get_state());
				return true;
			}
		}
		return false;
	}
	// finally state is mouse or drag
	if (e.get_kind() == cgv::gui::EID_KEY) {
		const auto& vrke = reinterpret_cast<const cgv::gui::vr_key_event&>(e);
		const auto& vr_state = vrke.get_state();
		int ci = vrke.get_controller_index();
		const auto& ctrl_state = vr_state.controller[ci];
		int bi = 0;
		switch (vrke.get_key()) {
		case vr::VR_MENU:
			if (vrke.get_action() == cgv::gui::KA_RELEASE) {
				show_screen = false;
				hide();
				if (screen_capture) {
					screen_capture = false;
					on_set(&screen_capture);
				}
				request.request = cgv::nui::focus_change_action::detach;
				request.demand.attach.level = cgv::nui::focus_level::hid;
			}
			return true;
		case vr::VR_DPAD_RIGHT:
			++bi;
		case vr::VR_DPAD_DOWN:
			++bi;
		case vr::VR_DPAD_LEFT:
			//std::cout << "mouse button " << bi << (vrke.get_action() == cgv::gui::KA_PRESS ? "pressed" : "released") << std::endl;
			if (vrke.get_action() == cgv::gui::KA_PRESS) {
				cgv::os::send_mouse_button_event(bi, true);
				mouse_button_pressed[bi] = true;
				update_member(&mouse_button_pressed[bi]);
				drag_begin(request, true, original_config);
				state = state_enum::drag;
				update_member(&state);
				return true;
			}
			if (vrke.get_action() == cgv::gui::KA_RELEASE) {
				cgv::os::send_mouse_button_event(bi, false);
				mouse_button_pressed[bi] = false;
				update_member(&mouse_button_pressed[bi]);
				drag_end(request, original_config);
				state = state_enum::mouse;
				update_member(&state);
				return true;
			}
			break;
		}
		return false;
	}
	if (e.get_kind() == cgv::gui::EID_POSE) {
		if (dis_info.mode == cgv::nui::dispatch_mode::pointing) {
			const auto& di = reinterpret_cast<const cgv::nui::intersection_dispatch_info&>(dis_info);
			int x, y;
			compute_mouse_xy(screen_point = di.hit_point, x, y);
			cgv::os::set_mouse_cursor(x, y);
			return true;
		}
		return false;
	}
	return false;
}

bool vr_screen::place_screen(const vr::vr_kit_state& state)
{
	if (state.hmd.status != vr::VRS_TRACKED)
		return false;

	const mat34& hmd_pose = reinterpret_cast<const mat34&>(state.hmd.pose[0]);
	if (state.controller[hid_id.index].status == vr::VRS_TRACKED) {
		const mat34& hand_pose = cgv::math::pose_concat(
			cgv::math::pose_inverse(reinterpret_cast<const mat34&>(hmd_pose[0])),
			reinterpret_cast<const mat34&>(state.controller[hid_id.index].pose[0])
		);
		vec3 z = hand_pose.col(2);
		vec3 z0 = start_placement_pose.col(2);
		vec3 x = hand_pose.col(0);
		vec3 x0 = start_placement_pose.col(0);
		vec3 y = hand_pose.col(1);
		vec3 y0 = start_placement_pose.col(1);
		vec3 x1 = normalize(cross(y0, z));
		vec3 y1 = cross(z, x1);
		float angle = atan2(dot(x, y1), dot(x, x1));
		screen_scale = exp(angle);
		screen_distance = 2.0f * exp(dot(z, y0));
		screen_x_offset = -screen_distance * dot(z, x0);
		update_member(&screen_distance);
		update_member(&screen_scale);
		update_member(&screen_x_offset);
	}
	screen_rotation = quat(reinterpret_cast<const mat3&>(hmd_pose));
	screen_reference = hmd_pose.col(3);
	place_screen();
	return true;
}

void vr_screen::place_screen()
{
	screen_center = screen_reference - screen_distance * screen_rotation.apply(vec3(0,0,1)) + screen_x_offset* screen_rotation.apply(vec3(1, 0, 0));
	screen_extent = screen_scale * vec2(screen_aspect, 1.0f);
	update_member(&screen_center[0]);
	update_member(&screen_center[1]);
	update_member(&screen_center[2]);
	update_member(&screen_extent[0]);
	update_member(&screen_extent[1]);
}

bool vr_screen::init(cgv::render::context& ctx)
{
	cgv::render::ref_sphere_renderer(ctx, 1);
	cgv::render::ref_rectangle_renderer(ctx, 1);
	return true;
}

void vr_screen::init_frame(cgv::render::context& ctx)
{
	if (screen_image_dirty.size()>0 && screen_image_dirty[0]) {
		if (!screen_texture.is_created())
			screen_texture.create(ctx, screen_images[0]);
		else
			screen_texture.replace(ctx, 0, 0, screen_images[0]);
		screen_image_dirty[0] = false;
	}
}

void vr_screen::clear(cgv::render::context& ctx)
{
	cgv::render::ref_sphere_renderer(ctx, -1);
	cgv::render::ref_rectangle_renderer(ctx, -1);
}

void vr_screen::draw(cgv::render::context& ctx)
{
	if (!show_screen)
		return;

	auto& rr = cgv::render::ref_rectangle_renderer(ctx);
	rr.set_render_style(screen_style);
	rr.set_position(ctx, screen_center);
	rr.set_rotation(ctx, screen_rotation);
	rr.set_extent(ctx, screen_extent);
	if (screen_texture.is_created()) {
		rr.set_texcoord(ctx, vec4(0, 1, 1, 0));
		screen_texture.enable(ctx);
	}
	rr.set_color(ctx, rgb(1, 1, 0));
	switch (state) {
	case state_enum::idle:	rr.set_border_color(ctx, rgb(0.3f, 0.3f, 0.3f)); break;
	case state_enum::place:	rr.set_border_color(ctx, rgb(0.3f, 0.8f, 0.3f)); break;
	case state_enum::mouse:	rr.set_border_color(ctx, rgb(0.8f, 0.3f, 0.3f)); break;
	case state_enum::drag:	rr.set_border_color(ctx, rgb(1.0f, 0.5f, 0.5f)); break;
	}
	rr.render(ctx, 0, 1);
	if (screen_texture.is_created())
		screen_texture.disable(ctx);

	if (state == state_enum::mouse || state == state_enum::drag) {
		auto& sr = cgv::render::ref_sphere_renderer(ctx);
		sr.set_position(ctx, screen_point);
		sr.set_color(ctx, rgb(1, 0, 0));
		sr.set_radius(ctx, 0.01f);
		sr.render(ctx, 0, 1);
	}
}

bool vr_screen::compute_intersection(const vec3& rectangle_center, const vec2& rectangle_extent,
	const vec3& ray_start, const vec3& ray_direction, float& ray_param)
{
	float t_result;
	vec3 p_result = ray_start - rectangle_center;
	// ray starts on rectangle
	if (fabs(ray_start[2]) < 0.000001f)
		t_result = 0;
	else {
		t_result = -p_result[2] / ray_direction[2];
		// intersection is before ray start
		if (t_result < 0)
			return false;
		p_result += t_result * ray_direction;
	}
	// check if ray plane intersection is inside of rectangle extent
	if (fabs(p_result[0]) > 0.5f * rectangle_extent[0] ||
		fabs(p_result[1]) > 0.5f * rectangle_extent[1])
		return false;
	ray_param = t_result;
	return true;
}

/// implement ray object intersection and return whether intersection was found and in this case set \c hit_param to ray parameter and optionally \c hit_normal to surface normal of intersection
bool vr_screen::compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx)
{
	if (!is_visible())
		return false;
	if (!compute_intersection(screen_center, screen_extent, screen_rotation, ray_start, ray_direction, hit_param))
		return false;
	hit_normal = screen_rotation.apply(vec3(0, 0, 1.0f));
	return true;
}

bool vr_screen::compute_intersection(
	const vec3& rectangle_center, const vec2& rectangle_extent, const quat& rectangle_rotation,
	const vec3& ray_start, const vec3& ray_direction, float& ray_param)
{
	vec3 ro = ray_start - rectangle_center;
	rectangle_rotation.inverse_rotate(ro);
	ro += rectangle_center;
	vec3 rd = ray_direction;
	rectangle_rotation.inverse_rotate(rd);
	return compute_intersection(rectangle_center, rectangle_extent, ro, rd, ray_param);
}

void vr_screen::create_gui()
{
	add_member_control(this, "show", show_screen, "toggle");
	add_member_control(this, "enabled", enabled, "toggle");
	add_member_control(this, "aspect", screen_aspect, "value_slider", "min=0.1;max=10;log=true;ticks=true");
	add_member_control(this, "capture", screen_capture, "toggle");
	if (begin_tree_node("interaction", state)) {
		align("\a");
		add_member_control(this, "state", state, "dropdown", "enums='idle,place,mouse,drag'");
		add_member_control(this, "left mouse button", mouse_button_pressed[0], "toggle");
		add_member_control(this, "middle mouse button", mouse_button_pressed[1], "toggle");
		add_member_control(this, "right mouse button", mouse_button_pressed[2], "toggle");
		align("\b");
		end_tree_node(state);
	}
	if (begin_tree_node("placement", screen_reference, true)) {
		align("\a");
		add_member_control(this, "distance", screen_distance, "value_slider", "min=0.1;max=10;log=true;ticks=true");
		add_member_control(this, "scale", screen_scale, "value_slider", "min=0.1;max=10;log=true;ticks=true");
		add_member_control(this, "x_offset", screen_x_offset, "value_slider", "min=-2;max=2;ticks=true");
		add_member_control(this, "center_x", screen_center[0], "value_slider", "min=-2;max=2;ticks=true");
		add_member_control(this, "center_y", screen_center[1], "value_slider", "min=-2;max=2;ticks=true");
		add_member_control(this, "center_z", screen_center[2], "value_slider", "min=-2;max=2;ticks=true");
		align("\b");
		end_tree_node(screen_reference);
	}
	if (begin_tree_node("style", screen_style)) {
		align("\a");
		add_gui("screen_style", screen_style);
		align("\b");
		end_tree_node(screen_style);
	}
}

	}
}