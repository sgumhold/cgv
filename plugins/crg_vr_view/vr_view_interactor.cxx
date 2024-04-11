#include "vr_view_interactor.h"
#include "vr_render_helpers.h"
#include <cgv/render/attribute_array_binding.h>
#include <cgv/render/shader_program.h>
#include <cgv/gui/trigger.h>
#include <cgv/signal/rebind.h>
#include <cgv/math/ftransform.h>
#include <cgv/base/import.h>
#include <cgv/math/inv.h>
#include <cgv/math/pose.h>
#include <iostream>
#include <sstream>
#include <vr/vr_kit.h>
#include <vr/vr_driver.h>
#include <cg_vr/vr_events.h>
#include <cg_vr/vr_calib.h>

cgv::reflect::enum_reflection_traits<VRkitVisType> get_reflection_traits(const VRkitVisType& gm)
{
	return cgv::reflect::enum_reflection_traits<VRkitVisType>("none,sphere,mesh,both");
}

///
vr_view_interactor::vr_view_interactor(const char* name) : stereo_view_interactor(name),
	fence_color1(0,0,1), fence_color2(1,1,0)
{
	pose_query = 2;
	blit_aspect_scale = 1;
	none_separate_view = 3;
	head_tracker_orientation.identity();
	head_tracker_position = vec3(0.0f);
	tracking_origin = vec3(0.0f);
	tracking_rotation_origin = vec3(0.0f);
	tracking_rotation = 0;
	head_tracker = -1;
	debug_vr_events = false;
	separate_view = true;
	dont_render_kits = false;
	blit_vr_views = true;
	blit_width = 160;
	event_flags = cgv::gui::VREventTypeFlags(cgv::gui::VRE_ALL);
	rendered_display_ptr = 0;
	rendered_eye = 0;
	rendered_display_index = -1;

	fence_frequency = 5;
	fence_line_width = 3;
	vis_type = hmd_vis_type = controller_vis_type = tracker_vis_type = base_vis_type = VVT_MESH;

	mesh_scales[0] = mesh_scales[1] = mesh_scales[2] = mesh_scales[3] = 1;

	show_action_zone = false;
	current_vr_handle = 0;
	current_vr_handle_index = -1;
	kit_enum_definition = "enums='none=-1'";

	brs.map_color_to_material = cgv::render::CM_COLOR;
	srs.map_color_to_material = cgv::render::CM_COLOR;
	srs.blend_width_in_pixel = 0;

	cgv::signal::connect(cgv::gui::ref_vr_server().on_device_change, this, &vr_view_interactor::on_device_change);
	cgv::signal::connect(cgv::gui::ref_vr_server().on_status_change, this, &vr_view_interactor::on_status_change);
}

/// set whether vr events should be printed to the console window
void vr_view_interactor::enable_vr_event_debugging(bool enable)
{
	if (debug_vr_events != enable) {
		debug_vr_events = enable;
		on_set(&debug_vr_events);
	}
}

/// set whether to draw separate view
void vr_view_interactor::draw_separate_view(bool do_draw)
{
	if (separate_view != do_draw) {
		separate_view = do_draw;
		on_set(&separate_view);
	}
}

/// whether to draw vr kits
void vr_view_interactor::draw_vr_kits(bool do_draw)
{
	if ((vis_type != VVT_NONE) != do_draw) {
		vis_type = do_draw ? VVT_MESH : VVT_NONE;
		on_set(&vis_type);
	}
}

/// set whether to draw controllers
void vr_view_interactor::draw_vr_controllers(bool do_draw)
{
	if ((controller_vis_type != VVT_NONE) != do_draw) {
		controller_vis_type = do_draw ? VVT_MESH : VVT_NONE;
		on_set(&controller_vis_type);
	}	
}

/// whether to draw action zone
void vr_view_interactor::draw_action_zone(bool do_draw)
{
	if (show_action_zone != do_draw) {
		show_action_zone = do_draw;
		on_set(&show_action_zone);
	}
}
/// enable vr view blitting
void vr_view_interactor::enable_blit_vr_views(bool enable)
{
	if (blit_vr_views != enable) {
		blit_vr_views = enable;
		on_set(&blit_vr_views);
	}
}
/// set the width with which vr views are blit
void vr_view_interactor::set_blit_vr_view_width(int width)
{
	if (blit_width != width) {
		blit_width = width;
		on_set(&blit_width);
	}
}

/// return a pointer to the state of the current vr kit
const vr::vr_kit_state* vr_view_interactor::get_current_vr_state() const
{
	if (current_vr_handle_index >= 0 && current_vr_handle_index < int(kit_states.size()))
		return &kit_states[current_vr_handle_index];
	return 0;
}

/// return a pointer to the current vr kit
vr::vr_kit* vr_view_interactor::get_current_vr_kit() const
{
	if (current_vr_handle_index >= 0 && current_vr_handle_index < int(kit_states.size()))
		return get_vr_kit_from_index(current_vr_handle_index);
	return 0;
}

vr_view_interactor::dvec3 vr_view_interactor::get_view_dir_of_kit(int vr_kit_idx) const
{
	if (vr_kit_idx == -1)
		vr_kit_idx = current_vr_handle_index;
	if (vr_kit_idx < 0 || vr_kit_idx >= (int) kit_states.size())
		return get_view_dir();
	return -reinterpret_cast<const vec3&>(kit_states[vr_kit_idx].hmd.pose[6]);
}

vr_view_interactor::dvec3 vr_view_interactor::get_view_up_dir_of_kit(int vr_kit_idx) const
{
	if (vr_kit_idx == -1)
		vr_kit_idx = current_vr_handle_index;
	if (vr_kit_idx < 0 || vr_kit_idx >= (int) kit_states.size()) {
		// ensure that view up is orthogonal to view dir
		return cross(get_view_dir(), cross(get_view_up_dir(), get_view_dir()));
	}
	return reinterpret_cast<const vec3&>(kit_states[vr_kit_idx].hmd.pose[3]);
}

vr_view_interactor::dvec3 vr_view_interactor::get_eye_of_kit(int eye, int vr_kit_idx) const
{
	if (vr_kit_idx == -1)
		vr_kit_idx = current_vr_handle_index;
	if (vr_kit_idx < 0 || vr_kit_idx >= (int) kit_states.size())
		return get_eye();
	return reinterpret_cast<const vec3&>(kit_states[vr_kit_idx].hmd.pose[9]);
}

/// query the currently set event type flags
cgv::gui::VREventTypeFlags vr_view_interactor::get_event_type_flags() const
{
	return event_flags;
}
/// set the event type flags of to be emitted events
void vr_view_interactor::set_event_type_flags(cgv::gui::VREventTypeFlags flags)
{
	event_flags = flags;
	on_set(&event_flags);
}

///
void vr_view_interactor::on_status_change(void* handle, int controller_index, vr::VRStatus old_status, vr::VRStatus new_status)
{
	post_redraw();
	if (debug_vr_events)
		std::cout << "on status change(" << handle << ","
		<< controller_index << "," << vr::get_status_string(old_status)
		<< "," << vr::get_status_string(new_status) << ")" << std::endl;
}

///
void vr_view_interactor::on_device_change(void* handle, bool attach)
{
	if (attach)
		new_kits.push_back(handle);
	else
		old_kits.push_back(handle);
	post_redraw();
	if (debug_vr_events)
		std::cout << "on device change(" << handle << ","
		<< (attach?"attach":"detach") << ")" << std::endl;
}

/// perform driver calibration
void vr_view_interactor::calibrate_driver()
{
	vr::vr_kit* kit_ptr = get_current_vr_kit();
	if (!kit_ptr)
		return;
	const vr::vr_driver* driver_ptr = kit_ptr->get_driver();
	float calibration_matrix[12];
	mat34& C = reinterpret_cast<mat34&>(calibration_matrix[0]);
	pose_orientation(C) = cgv::math::rotate3<float>(tracking_rotation, vec3(0, 1, 0));
	pose_position(C) = tracking_origin - pose_orientation(C)*tracking_rotation_origin;
	set_driver_calibration_matrix(const_cast<vr::vr_driver*>(driver_ptr), calibration_matrix);
	const_cast<vr::vr_driver*>(driver_ptr)->enable_calibration_transformation();
}

/// return the type name 
std::string vr_view_interactor::get_type_name() const
{
	return "vr_view_interactor";
}
/// 
void vr_view_interactor::on_set(void* member_ptr)
{
	if (member_ptr == &calibration_file_path) {
		if (ref_tree_node_visible_flag(calibration_file_path)) {
			cgv::gui::ref_vr_calibration().update_calibration_info();
			cgv::gui::ref_vr_calibration().write_calibration(calibration_file_path);
		}
		else
			cgv::gui::ref_vr_calibration().read_calibration(calibration_file_path);
	}
	if (member_ptr == &hmd_mesh_file_name)
		vr::set_vrmesh_file_name(vr::VRM_HMD, hmd_mesh_file_name);
	if (member_ptr == &controller_mesh_file_name)
		vr::set_vrmesh_file_name(vr::VRM_CONTROLLER, controller_mesh_file_name);
	if (member_ptr == &tracker_mesh_file_name)
		vr::set_vrmesh_file_name(vr::VRM_TRACKER, tracker_mesh_file_name);
	if (member_ptr == &base_mesh_file_name) {
		vr::set_vrmesh_file_name(vr::VRM_BASE, base_mesh_file_name);
		update_member(&base_mesh_file_name);
	}

	if (member_ptr == &vis_type) {
		hmd_vis_type = controller_vis_type = tracker_vis_type = vis_type;
		update_member(&hmd_vis_type);
		update_member(&controller_vis_type);
		update_member(&tracker_vis_type);
		update_member(&base_vis_type);
	}

	if (member_ptr == &current_vr_handle_index) {
		if (current_vr_handle_index == -1) {
			current_vr_handle = 0;
			if (!separate_view) {
				separate_view = true;
				update_member(&separate_view);
			}
		}
		else
			if (current_vr_handle_index < int(kits.size()))
				current_vr_handle = kits[current_vr_handle_index];
	}
	if (member_ptr == &tracking_rotation ||
		(member_ptr >= &tracking_origin && member_ptr < &tracking_origin + 1) ||
		(member_ptr >= &tracking_rotation_origin && member_ptr < &tracking_rotation_origin + 1)) {
		calibrate_driver();
	}
	if (member_ptr == &head_tracker) {
		if (current_vr_handle_index >= 0) {
			const auto& cs = kit_states[current_vr_handle_index].controller[head_tracker];
			if (cs.status == vr::VRS_TRACKED) {
				const mat3& O = reinterpret_cast<const mat3&>(cs.pose[0]);
				const vec3& p = reinterpret_cast<const vec3&>(cs.pose[9]);
				mat3 V;
				vec3& x = reinterpret_cast<vec3&>(V[0]);
				vec3& y = reinterpret_cast<vec3&>(V[3]);
				vec3& z = reinterpret_cast<vec3&>(V[6]);
				vec3  e = get_eye();
				put_coordinate_system(x, y, z);
				head_tracker_orientation = V * transpose(O);
				head_tracker_position = e - head_tracker_orientation * p;
			}
		}
	}
	stereo_view_interactor::on_set(member_ptr);
}

/// overload to stream help information to the given output stream
void vr_view_interactor::stream_help(std::ostream& os)
{
	os << "vr_view_interactor: Ctrl-0|1|2|3 to select player; Ctrl-Space to toggle draw separate view\n"
	   << "   Shift-Ctrl-0|1|2|3 to identify current focus point with conroller or tracker\n";
	stereo_view_interactor::stream_help(os);
}

/// overload to show the content of this object
void vr_view_interactor::stream_stats(std::ostream& os)
{
	stereo_view_interactor::stream_stats(os);
}

bool vr_view_interactor::init(cgv::render::context& ctx)
{
	cgv::render::ref_sphere_renderer(ctx, 1);

#ifndef _DEBUG
	// set mesh file names only in release configuration
	if (vr::get_vrmesh_file_name(vr::VRM_HMD).empty())
		vr::set_vrmesh_file_name(vr::VRM_HMD, "generic_hmd.obj");
	if (vr::get_vrmesh_file_name(vr::VRM_CONTROLLER).empty())
		vr::set_vrmesh_file_name(vr::VRM_CONTROLLER, "vr_controller_vive_1_5.obj");
	if (vr::get_vrmesh_file_name(vr::VRM_TRACKER).empty()) {
		vr::set_vrmesh_file_name(vr::VRM_TRACKER, "HTC_Vive_Tracker_2017.obj");
		mesh_scales[vr::VRM_TRACKER] = 0.001f;
	}
	if (vr::get_vrmesh_file_name(vr::VRM_BASE).empty())
		vr::set_vrmesh_file_name(vr::VRM_BASE, "lh_basestation_vive.obj");
#endif
	return stereo_view_interactor::init(ctx);
}

void vr_view_interactor::clear(cgv::render::context& ctx)
{
	cgv::render::ref_sphere_renderer(ctx, -1);
}

/// factored our vr event handling
bool vr_view_interactor::handle_vr_events(cgv::gui::event& e)
{
	return false;
	if (e.get_kind() == cgv::gui::EID_KEY) {
		auto& vrke = reinterpret_cast<cgv::gui::vr_key_event&>(e);
		if (vrke.get_key() == vr::VR_INPUT0 && vrke.get_controller_index() == 1) {
			if (vrke.get_action() == cgv::gui::KA_PRESS) {
				start_pose = reinterpret_cast<const mat34&>(*vrke.get_state().controller[1].pose);
			}
			else if (vrke.get_action() == cgv::gui::KA_RELEASE) {
				mat34 end_pose = reinterpret_cast<const mat34&>(*vrke.get_state().controller[1].pose);
				tracking_origin += pose_position(end_pose) - pose_position(start_pose);
			}
			return true;
		}
	}
}

/// overload and implement this method to handle events
bool vr_view_interactor::handle(cgv::gui::event& e)
{
	if ((e.get_flags() & cgv::gui::EF_VR) != 0) {
		if (debug_vr_events) {
			e.stream_out(std::cout);
			std::cout << std::endl;
		}
		return handle_vr_events(e);
	}

	if (head_tracker != -1) {
		if ( ((e.get_flags() & cgv::gui::EF_VR) != 0) && (e.get_kind() == cgv::gui::EID_POSE) ) {
			cgv::gui::vr_pose_event& vrpe = dynamic_cast<cgv::gui::vr_pose_event&>(e);
			if (vrpe.get_trackable_index() == head_tracker) {
				const mat3& O = reinterpret_cast<const mat3&>(vrpe.get_orientation());
				const vec3& p = reinterpret_cast<const vec3&>(vrpe.get_position());
				mat3 V = head_tracker_orientation * O;
				set_view_up_dir(V.col(1));
				set_view_dir(-V.col(2));

				vec3 dv = get_focus() - get_eye();
				vec3 e = head_tracker_orientation * p + head_tracker_position;
				set_focus(e+dv);
			}
		}
	}
	if (e.get_kind() == cgv::gui::EID_KEY) {
		cgv::gui::key_event& ke = static_cast<cgv::gui::key_event&>(e);
		if ((ke.get_action() == cgv::gui::KA_PRESS)) {
			if (ke.get_modifiers() == cgv::gui::EM_CTRL) {
				if (ke.get_key() >= '0' && ke.get_key() < '4') {
					unsigned player_index = ke.get_key() - '0';
					if (player_index < kits.size()) {
						current_vr_handle_index = player_index;
						current_vr_handle = kits[player_index];
						update_member(&current_vr_handle_index);
						return true;
					}
				}
				if (ke.get_key() == cgv::gui::KEY_Space) {
					separate_view = !separate_view;
					on_set(&separate_view);
					return true;
				}
			}
			else if (ke.get_modifiers() == cgv::gui::EM_CTRL + cgv::gui::EM_SHIFT) {
				if (ke.get_key() >= '0' && ke.get_key() < '1'+vr::max_nr_controllers) {
					int ci = ke.get_key() - '0';
					if (ci == vr::max_nr_controllers)
						ci = 0;
					if (current_vr_handle_index >= 0) {
						vr::vr_kit_state& state = kit_states[current_vr_handle_index];
						if (state.controller[ci].status == vr::VRS_TRACKED) {
							// p = R * (q-r) + t 
							// f = R * (q-r') + t'
							// p - f = R*(r'-r)+t-t'
							// t' = f
							// p - f = R*(r'-r)+t-f
							// r' = r = R^*(p - t) 
							vec3& p = reinterpret_cast<vec3&>(state.controller[ci].pose[9]);
							mat3 invR = cgv::math::rotate3<float>(-tracking_rotation, vec3(0, 1, 0));
							tracking_rotation_origin += invR* (p - tracking_origin);
							tracking_origin = get_focus();
							p = get_focus();
							for (int i = 0; i < 3; ++i) {
								update_member(&tracking_origin[i]);
								update_member(&tracking_rotation_origin[i]);
							}
							calibrate_driver();
						}
					}
				}
			}
		}
	}
	return stereo_view_interactor::handle(e);
}


/// this method is called in one pass over all drawables after drawing
void vr_view_interactor::finish_frame(cgv::render::context& ctx)
{
	stereo_view_interactor::finish_frame(ctx);
}

void vr_view_interactor::after_finish(cgv::render::context& ctx)
{
	stereo_view_interactor::after_finish(ctx);
	if (ctx.get_render_pass() == cgv::render::RP_MAIN) {
		if (rendered_display_ptr) {
			rendered_display_ptr->disable_fbo(rendered_eye);
			ctx.recover_from_external_frame_buffer_change(fbo_handle);
			ctx.recover_from_external_viewport_change(cgv_viewport);
			if (!separate_view) {

				int x0 = 0, width = ctx.get_width(), eye = 0, eye_end = 2;
				switch (none_separate_view) {
				case 1: eye_end = 1; break;
				case 2: eye = 1; break;
				case 3: width /= 2; break;
				}
				for (; eye < eye_end; ++eye) {
					rendered_display_ptr->blit_fbo(eye, x0, 0, width, ctx.get_height());
					x0 += width;
				}
			}
			rendered_eye = 0;
			rendered_display_ptr = 0;
			rendered_display_index = -1;
		}
		// submit frames to active vr kits and blit vr kit views in main framebuffer if activated
		if (!dont_render_kits) {
			int y0 = 0;
			for (size_t ki = 0; ki < kits.size(); ++ki) {
				// check if kit is attached and its pointer valid
				if (kit_states[ki].hmd.status == vr::VRS_DETACHED)
					continue;
				vr::vr_kit* kit_ptr = get_vr_kit_from_index((int)ki);
				if (!kit_ptr)
					continue;

				if (blit_vr_views && (separate_view || ki != current_vr_handle_index)) {
					int x0 = 0;
					int blit_height = (int)(blit_width * kit_ptr->get_height() / (blit_aspect_scale*kit_ptr->get_width()));
					for (int eye = 0; eye < 2; ++eye) {
						kit_ptr->blit_fbo(eye, x0, y0, blit_width, blit_height);
						x0 += blit_width + 5;
					}
					y0 += blit_height + 5;
				}
				kit_ptr->submit_frame();
			}
		}
		if (current_vr_handle)
			post_redraw();
	}
}

vr::vr_kit* vr_view_interactor::get_vr_kit_from_index(int i) const
{
	return vr::get_vr_kit(kits[i]);
}

void vr_view_interactor::configure_kits()
{
	bool update_kits = false;
	void* last_current_vr_handle = current_vr_handle;
	// check for old kits, destruct their render objects, unregister them and delete them
	while (!old_kits.empty()) {
		if (current_vr_handle == old_kits.back())
			current_vr_handle = 0;
		vr::vr_kit* kit_ptr = vr::get_vr_kit(old_kits.back());
		if (kit_ptr) {
			if (kit_ptr->fbos_initialized())
				kit_ptr->destruct_fbos();
			if (!vr::unregister_vr_kit(old_kits.back(), kit_ptr)) {
				std::cerr << "unable to unregister removed kit <" << old_kits.back() << ">" << std::endl;
			}
			delete kit_ptr;
		}
		auto iter = std::find(kits.begin(), kits.end(), old_kits.back());
		if (iter != kits.end()) {
			kits.erase(iter);
			update_kits = true;
		}
		old_kits.pop_back();
	}
	if (current_vr_handle == 0 && !kits.empty())
		current_vr_handle = kits.front();
	// check for new kits and initialize their render objects in current context
	while (!new_kits.empty()) {
		vr::vr_kit* kit_ptr = vr::get_vr_kit(new_kits.back());
		if (kit_ptr) {
			if (!kit_ptr->fbos_initialized())
				if (kit_ptr->init_fbos()) {
					std::cout << "initialized fbos of " << kit_ptr->get_name() << " in context " << (void*)get_context() << std::endl;
					if (current_vr_handle == 0)
						current_vr_handle = new_kits.back();
				}
			kits.push_back(new_kits.back());
			update_kits = true;
		}
		new_kits.pop_back();
	}
	kit_states.resize(kits.size());
	// 
	if (update_kits) {
		kit_enum_definition = "enums='none=-1";
		for (auto handle : kits) {
			vr::vr_kit* kit_ptr = vr::get_vr_kit(handle);
			std::string kit_name;
			if (kit_ptr)
				kit_name = kit_ptr->get_name();
			else {
				std::stringstream ss;
				ss << handle;
				kit_name = ss.str();
			}
			kit_enum_definition += ";";
			kit_enum_definition += kit_name;
		}
		kit_enum_definition += "'";
		if (find_control(current_vr_handle_index))
			find_control(current_vr_handle_index)->multi_set(kit_enum_definition);
		std::cout << kit_enum_definition << std::endl;
	}
	if (update_kits || current_vr_handle != last_current_vr_handle) {
		if (current_vr_handle == 0)
			current_vr_handle_index = -1;
		else {
			for (unsigned i = 0; i < kits.size(); ++i)
				if (kits[i] == current_vr_handle) {
					current_vr_handle_index = i;
					if (calibration_file_path.empty())
						calibrate_driver();
					break;
				}
		}
		update_member(&current_vr_handle_index);
	}
}

// query vr state
void vr_view_interactor::query_vr_states()
{
	// query states
	vr::vr_kit* current_kit_ptr = 0;
	if (current_vr_handle_index >= 0) {
		current_kit_ptr = get_vr_kit_from_index(current_vr_handle_index);
		if (current_kit_ptr) {
			vr::vr_kit_state& state = kit_states[current_vr_handle_index];
			current_kit_ptr->query_state(state, 2);
			cgv::gui::ref_vr_server().check_new_state(current_vr_handle, state, cgv::gui::trigger::get_current_time(), event_flags);
		}
	}
	for (unsigned i = 0; i < kits.size(); ++i) {
		vr::vr_kit* kit_ptr = get_vr_kit_from_index(i);
		if (!kit_ptr)
			continue;
		if (kit_ptr == current_kit_ptr)
			continue;
		kit_ptr->query_state(kit_states[i], 1);
		if (kit_states[i].hmd.status == vr::VRS_DETACHED)
			continue;
		cgv::gui::ref_vr_server().check_new_state(kits[i], kit_states[i], cgv::gui::trigger::get_current_time(), event_flags);
	}
}

// render the views for the kits in nested render passes
void vr_view_interactor::render_vr_kits(cgv::render::context& ctx)
{
	cgv::render::RenderPassFlags rpf = ctx.get_render_pass_flags();
	// first render all but current vr kit views
	for (rendered_display_index = 0; rendered_display_index<int(kits.size()); ++rendered_display_index) {
		if (rendered_display_index == current_vr_handle_index)
			continue;
		rendered_display_ptr = get_vr_kit_from_index(rendered_display_index);
		if (!rendered_display_ptr)
			continue;
		if (kit_states[rendered_display_index].hmd.status == vr::VRS_DETACHED)
			continue;
		void* fbo_handle;
		ivec4 cgv_viewport;
		for (rendered_eye = 0; rendered_eye < 2; ++rendered_eye) {
			rendered_display_ptr->enable_fbo(rendered_eye);
			ctx.announce_external_frame_buffer_change(fbo_handle);
			ctx.announce_external_viewport_change(cgv_viewport);
			ctx.render_pass(cgv::render::RP_USER_DEFINED, cgv::render::RenderPassFlags(rpf & ~cgv::render::RPF_HANDLE_SCREEN_SHOT), this);
			rendered_display_ptr->disable_fbo(rendered_eye);
			ctx.recover_from_external_viewport_change(cgv_viewport);
			ctx.recover_from_external_frame_buffer_change(fbo_handle);
		}
	}
	// then render current vr kit 
	rendered_display_index = current_vr_handle_index;
	rendered_display_ptr = get_vr_kit_from_index(rendered_display_index);
	if (rendered_display_ptr && kit_states[rendered_display_index].hmd.status != vr::VRS_DETACHED) {
		void* fbo_handle;
		ivec4 cgv_viewport;
		//rendered_eye: 0...monitor,1...left eye,2...right eye
		for (rendered_eye = 0; rendered_eye < 2; ++rendered_eye) {
			rendered_display_ptr->enable_fbo(rendered_eye);
			ctx.announce_external_frame_buffer_change(fbo_handle);
			ctx.announce_external_viewport_change(cgv_viewport);
			// we break here in the case of no separate to use main render pass to render second eye of current vr kit
			if (rendered_eye == 1 && !separate_view) {
				this->fbo_handle = fbo_handle;
				this->cgv_viewport = cgv_viewport;
				break;
			}
			ctx.render_pass(cgv::render::RP_USER_DEFINED, cgv::render::RenderPassFlags(rpf & ~cgv::render::RPF_HANDLE_SCREEN_SHOT));
			rendered_display_ptr->disable_fbo(rendered_eye);
			ctx.recover_from_external_viewport_change(cgv_viewport);
			ctx.recover_from_external_frame_buffer_change(fbo_handle);
		}
	}
	else {
		rendered_display_ptr = 0;
		rendered_display_index = -1;
	}
	if (separate_view) {
		rendered_display_ptr = 0;
		rendered_display_index = -1;
	}
}

/// this method is called in one pass over all drawables before the draw method
void vr_view_interactor::init_frame(cgv::render::context& ctx)
{
	if (ctx.get_render_pass() == cgv::render::RP_MAIN) {
		// update current vr kits
		configure_kits();
		// perform rendering from the vr kits
		if (kits.size() > 0) {
			// query vr state
			query_vr_states();
			// render the views for the kits in nested render passes
			if (!dont_render_kits)
				render_vr_kits(ctx);
		}
	}
	// set model view projection matrices for currently rendered eye of rendered vr kit
	if (rendered_display_ptr) {
		vr::vr_kit* rendered_vr_kit = get_rendered_vr_kit();
		compute_clipping_planes(z_near_derived, z_far_derived, clip_relative_to_extent);
		ctx.set_projection_matrix(vr::get_eye_projection_transform(rendered_vr_kit, kit_states[rendered_display_index], float(z_near_derived), float(z_far_derived), rendered_eye));
		ctx.set_modelview_matrix(vr::get_world_to_eye_transform(rendered_vr_kit, kit_states[rendered_display_index], rendered_eye));
	}
	else {
		// use standard rendering for separate view or if no vr kit is available
		if (kits.empty() || separate_view)
			stereo_view_interactor::init_frame(ctx);
	}
}

void vr_view_interactor::add_trackable_spheres(const float* pose, int i, std::vector<vec4>& spheres, std::vector<rgb>& sphere_colors)
{
	const mat3& R_ci = reinterpret_cast<const mat3&>(pose[0]);
	const vec3& p_ci = reinterpret_cast<const vec3&>(pose[9]);
	spheres.push_back(vec4(p_ci, 0.04f));
	spheres.push_back(vec4(p_ci + 0.05f*R_ci.col(0), 0.01f));
	spheres.push_back(vec4(p_ci - 0.05f*R_ci.col(0), 0.01f));
	spheres.push_back(vec4(p_ci + 0.05f*R_ci.col(1), 0.01f));
	spheres.push_back(vec4(p_ci - 0.05f*R_ci.col(1), 0.01f));
	spheres.push_back(vec4(p_ci + 0.05f*R_ci.col(2), 0.01f));
	spheres.push_back(vec4(p_ci - 0.05f*R_ci.col(2), 0.01f));
	sphere_colors.push_back(rgb(0.5f + (1 - i)*0.5f, 0.5f, 0.5f + 0.5f*i));
	sphere_colors.push_back(rgb(1, 0, 0));
	sphere_colors.push_back(rgb(1, 0.5f, 0.5f));
	sphere_colors.push_back(rgb(0, 1, 0));
	sphere_colors.push_back(rgb(0.5f, 1, 0.5f));
	sphere_colors.push_back(rgb(0, 0, 1));
	sphere_colors.push_back(rgb(0.5f, 0.5f, 1));
}
/// 
void vr_view_interactor::draw_vr_kits(cgv::render::context& ctx)
{
	std::vector<vec4> spheres;
	std::vector<rgb> sphere_colors;
	cgv::render::mesh_render_info* MI_hmd_ptr = 0;
	cgv::render::mesh_render_info* MI_controller_ptr = 0;
	cgv::render::mesh_render_info* MI_tracker_ptr = 0;
	cgv::render::mesh_render_info* MI_base_ptr = 0;
	std::set<const vr::vr_driver*> driver_set;
	for (int i = 0; i < (int)kits.size(); ++i) {
		// skip removed kits
		vr::vr_kit* kit_ptr = get_vr_kit_from_index(i);
		if (!kit_ptr)
			continue;
		// extract drivers of which the bases should be drawn
		if (base_vis_type != VVT_NONE)
			driver_set.insert(kit_ptr->get_driver());
		// ignore completely hidden vr kits
		if (hmd_vis_type == VVT_NONE && controller_vis_type == VVT_NONE && tracker_vis_type == VVT_NONE)
			continue;
		// determine state of vr kit and skip kit if no state is available
		vr::vr_kit_state state;
		vr::vr_kit_state* state_ptr = &state;
		if (i == current_vr_handle_index)
			state_ptr = &kit_states[current_vr_handle_index];
		else 
			if (!kit_ptr->query_state(state, 1))
				continue;
		// render other hmds
		if (kit_ptr != rendered_display_ptr) {
			// either as mesh
			if ((hmd_vis_type & VVT_MESH) != 0) {
				if (!MI_hmd_ptr) {
					MI_hmd_ptr = vr::get_vrmesh_render_info(ctx, vr::VRM_HMD);
					// try to read meshes and turn off mesh rendering and potentially switch to sphere rendering in case meshes are not available
					if (MI_hmd_ptr) {
						hmd_mesh_file_name = vr::get_vrmesh_file_name(vr::VRM_HMD);
						update_member(&hmd_mesh_file_name);
					}
					else {
						hmd_vis_type = VVT_SPHERE;
						on_set(&hmd_vis_type);
					}
				}
				if (MI_hmd_ptr != 0) {
					ctx.push_modelview_matrix();
					ctx.mul_modelview_matrix(
						cgv::math::pose4<float>(reinterpret_cast<const mat34&>(state_ptr->hmd.pose[0]))*
						cgv::math::translate4<float>(0, 0.1f, -0.1f)*
						cgv::math::scale4<float>(vec3(mesh_scales[vr::VRM_HMD]))
					);
					MI_hmd_ptr->draw_all(ctx);
					ctx.pop_modelview_matrix();
				}
			}
			// and or as spheres
			if ((hmd_vis_type & VVT_SPHERE) != 0) {
				float left_eye_to_head[12];
				float right_eye_to_head[12];
				kit_ptr->put_eye_to_head_matrix(0, left_eye_to_head);
				kit_ptr->put_eye_to_head_matrix(1, right_eye_to_head);
				const mat3& R_w_h = reinterpret_cast<const mat3&>(state_ptr->hmd.pose[0]);
				const vec3& p_w_h = reinterpret_cast<const vec3&>(state_ptr->hmd.pose[9]);
				const mat3& R_h_l = reinterpret_cast<const mat3&>(left_eye_to_head[0]);
				const vec3& p_h_l = reinterpret_cast<const vec3&>(left_eye_to_head[9]);
				const mat3& R_h_r = reinterpret_cast<const mat3&>(right_eye_to_head[0]);
				const vec3& p_h_r = reinterpret_cast<const vec3&>(right_eye_to_head[9]);
				vec4 s_l(0, 0, 0, 0.012f);
				vec4 s_r = s_l;
				reinterpret_cast<vec3&>(s_l) = R_w_h * p_h_l + p_w_h;
				reinterpret_cast<vec3&>(s_r) = R_w_h * p_h_r + p_w_h;
				spheres.push_back(s_l);
				sphere_colors.push_back(rgb(1, 0, 0));
				spheres.push_back(s_r);
				sphere_colors.push_back(rgb(0, 0, 1));
			}
		}
		for (unsigned ci = 0; ci < vr::max_nr_controllers; ++ci) {
			if (state_ptr->controller[ci].status != vr::VRS_TRACKED || state_ptr->controller[ci].time_stamp == 0)
				continue;
			bool show_trackable_spheres;
			cgv::render::mesh_render_info* M_info = 0;
			float mesh_scale = 1;
			if (kit_ptr->get_device_info().controller[ci].type == vr::VRC_CONTROLLER) {
				show_trackable_spheres = (controller_vis_type & VVT_SPHERE) != 0;
				if ((controller_vis_type & VVT_MESH) != 0) {
					if (!MI_controller_ptr) {
						MI_controller_ptr = vr::get_vrmesh_render_info(ctx, vr::VRM_CONTROLLER);
						if (MI_controller_ptr) {
							controller_mesh_file_name = vr::get_vrmesh_file_name(vr::VRM_CONTROLLER);
							update_member(&controller_mesh_file_name);
						}
						else {
							controller_vis_type = VVT_SPHERE;
							on_set(&controller_vis_type);
						}
					}
					if (MI_controller_ptr) {
						M_info = MI_controller_ptr;
						mesh_scale = mesh_scales[vr::VRM_CONTROLLER];
					}
				}
			}
			else {
				show_trackable_spheres = (tracker_vis_type & VVT_SPHERE) != 0;
				if ((tracker_vis_type & VVT_MESH) != 0) {
					if (!MI_tracker_ptr) {
						MI_tracker_ptr = vr::get_vrmesh_render_info(ctx, vr::VRM_TRACKER);
						if (MI_tracker_ptr) {
							tracker_mesh_file_name = vr::get_vrmesh_file_name(vr::VRM_TRACKER);
							update_member(&tracker_mesh_file_name);
						}
						else {
							tracker_vis_type = VVT_SPHERE;
							on_set(&tracker_vis_type);
						}
					}
					if (MI_tracker_ptr) {
						M_info = MI_tracker_ptr;
						mesh_scale = mesh_scales[vr::VRM_TRACKER];
					}
				}
			}
			if (show_trackable_spheres)
				add_trackable_spheres(state_ptr->controller[ci].pose, i, spheres, sphere_colors);
			if (M_info) {
				ctx.push_modelview_matrix();
				ctx.mul_modelview_matrix(cgv::math::pose4<float>(reinterpret_cast<const mat34&>(state_ptr->controller[ci].pose[0])));
				ctx.mul_modelview_matrix(cgv::math::scale4<float>(vec3(mesh_scale)));
				M_info->draw_all(ctx);
				ctx.pop_modelview_matrix();
			}
		}
	}
	for (auto& dp : driver_set) {
		auto ss = dp->get_tracking_reference_states();
		for (const auto& s : ss) {
			if (s.second.status == vr::VRS_TRACKED) {
				if ((base_vis_type & VVT_SPHERE) != 0)
					add_trackable_spheres(s.second.pose, -1, spheres, sphere_colors);
				if ((base_vis_type & VVT_MESH) != 0) {
					if (!MI_base_ptr) {
						MI_base_ptr = vr::get_vrmesh_render_info(ctx, vr::VRM_BASE);
						if (MI_base_ptr) {
							base_mesh_file_name = vr::get_vrmesh_file_name(vr::VRM_BASE);
							update_member(&base_mesh_file_name);
						}
						else {
							base_vis_type = VVT_SPHERE;
							on_set(&base_vis_type);
						}
					}
					if (MI_base_ptr) {
						ctx.push_modelview_matrix();
						ctx.mul_modelview_matrix(cgv::math::pose4<float>(reinterpret_cast<const mat34&>(s.second.pose[0])));
						ctx.mul_modelview_matrix(cgv::math::scale4<float>(vec3(mesh_scales[vr::VRM_BASE])));
						MI_base_ptr->draw_all(ctx);
						ctx.pop_modelview_matrix();
					}
				}
			}
		}
	}
	if (!spheres.empty()) {
		cgv::render::sphere_renderer& sr = cgv::render::ref_sphere_renderer(ctx);
		sr.set_y_view_angle(float(get_y_view_angle()));
		sr.set_render_style(srs);
		sr.set_sphere_array(ctx, spheres);
		sr.set_color_array(ctx, sphere_colors);
		sr.render(ctx, 0, spheres.size());
	}
}

/// draw the action zone of the current vr kit
void vr_view_interactor::draw_action_zone(cgv::render::context& ctx)
{
	if (show_action_zone && current_vr_handle) {
		double time = cgv::gui::trigger::get_current_time();
		vr::vr_kit* kit_ptr = get_vr_kit_from_index(current_vr_handle_index);
		if (kit_ptr) {
			const vr::vr_driver* driver_ptr = kit_ptr->get_driver();
			if (driver_ptr) {
				float h = driver_ptr->get_action_zone_height();
				vec3 up_dir;
				driver_ptr->put_up_direction(&up_dir[0]);
				std::vector<float> boundary;
				driver_ptr->put_action_zone_bounary(boundary);
				size_t n = boundary.size() / 3;
				std::vector<vec3> G;
				G.resize(5 * n);
				unsigned i;
				for (i = 0; i < 5; ++i) {
					for (size_t j = 0; j < n; ++j) {
						vec3 p = reinterpret_cast<const vec3&>(boundary[3 * j]);
						p += 0.25f*i*h*up_dir;
						G[i*n + j] = p;
					}
				}
				glLineStipple(3, GLushort(0xF0F0));
				glEnable(GL_LINE_STIPPLE);
				glLineWidth(fence_line_width);
				auto& prog = ctx.ref_default_shader_program();
				int pos_idx = prog.get_position_index();
				cgv::render::attribute_array_binding::set_global_attribute_array(ctx, pos_idx, G);
				cgv::render::attribute_array_binding::enable_global_array(ctx, pos_idx);
				prog.enable(ctx);
				float lambda = float(cos(fence_frequency*time));
				lambda *= lambda;
				ctx.set_color((1 - lambda)*fence_color1 + lambda * fence_color2);
				for (i = 0; i < 5; ++i)
					glDrawArrays(GL_LINE_LOOP, GLint(i*n), (GLsizei)n);
				prog.disable(ctx);
				cgv::render::attribute_array_binding::disable_global_array(ctx, pos_idx);
				glDisable(GL_LINE_STIPPLE);
				glLineWidth(1.0f);
				post_redraw();
			}
		}
	}
}

/// 
void vr_view_interactor::draw(cgv::render::context& ctx)
{
	draw_vr_kits(ctx);
	draw_action_zone(ctx);
	stereo_view_interactor::draw(ctx);
}

/// you must overload this for gui creation
void vr_view_interactor::create_gui()
{
	add_member_control(this, "current vr kit", (cgv::type::DummyEnum&)current_vr_handle_index, "dropdown", kit_enum_definition);
	if (begin_tree_node("VR calibration", tracking_rotation, false, "level=2")) {
		align("\a");
		add_gui("calibration_file_path", calibration_file_path, "file_name",
			"open=true;open_title='open calibration file';filter='calib (cal):*.cal|all files:*.*';"
			"save=true;save_title='save calibration file';w=140");

		add_member_control(this, "tracking_rotation", tracking_rotation, "value_slider", "min=-180;max=180;ticks=true");
		if (begin_tree_node("translational", tracking_origin, false, "level=2")) {
			align("\a");
				add_decorator("origin", "heading", "level=3");
				add_gui("tracking_origin", tracking_origin, "", "gui_type='value_slider';options='min=-2;max=2;ticks=true'");
				add_decorator("rotation origin", "heading", "level=3");
				add_gui("tracking_rotation_origin", tracking_rotation_origin, "", "gui_type='value_slider';options='min=-2;max=2;ticks=true'");
			align("\b");
			end_tree_node(tracking_origin);
		}
		align("\b");
		end_tree_node(tracking_rotation);
	}
	if (begin_tree_node("VR rendering", separate_view, false, "level=2")) {
		align("\a");
		add_member_control(this, "scale", mesh_scales[0], "value", "w=42;align='B'", " ");
		add_gui("hmd_mesh_file_name", hmd_mesh_file_name, "file_name", "w=132;align='B';title='read tracker mesh from file';filter='mesh (obj):*.obj|all files:*.*'");
		add_member_control(this, "scale", mesh_scales[1], "value", "w=42;align='B'", " ");
		add_gui("controller_mesh_file_name", controller_mesh_file_name, "file_name", "w=132;align='B';title='read tracker mesh from file';filter='mesh (obj):*.obj|all files:*.*'");
		add_member_control(this, "scale", mesh_scales[2], "value", "w=42;align='B'", " ");
		add_gui("tracker_mesh_file_name", tracker_mesh_file_name, "file_name", "w=132;align='B';title='read tracker mesh from file';filter='mesh (obj):*.obj|all files:*.*'");
		add_member_control(this, "scale", mesh_scales[3], "value", "w=42;align='B'", " ");
		add_gui("base_mesh_file_name", base_mesh_file_name, "file_name", "w=132;align='B';title='read tracker mesh from file';filter='mesh (obj):*.obj|all files:*.*'");
		add_member_control(this, "separate_view", separate_view, "check");
		add_member_control(this, "none_separate_view", (cgv::type::DummyEnum&)none_separate_view, "dropdown", "enums='left=1,right=2,both=3'");
		add_member_control(this, "head_tracker", head_tracker, "value_slider", "min=-1;max=3");
		add_member_control(this, "dont_render_kits", dont_render_kits, "check");
		add_member_control(this, "blit_vr_views", blit_vr_views, "check");
		add_member_control(this, "blit_width", blit_width, "value_slider", "min=120;max=640;ticks=true;log=true");
		add_member_control(this, "blit_aspect_scale", blit_aspect_scale, "value_slider", "min=0.5;max=2;ticks=true;log=true");
		add_member_control(this, "show_action_zone", show_action_zone, "check");
		if (begin_tree_node("fence styles", fence_color1, false, "level=3")) {
			align("\a");
			add_member_control(this, "fence_color1", fence_color1);
			add_member_control(this, "fence_color2", fence_color2);
			add_member_control(this, "fence_line_width", fence_line_width, "value_slider", "min=1;max=20;ticks=true;log=true");
			add_member_control(this, "fence_frequency", fence_frequency, "value_slider", "min=0.1;max=10;ticks=true;log=true");
			align("\b");
			end_tree_node(fence_color1);
		}
		add_decorator("visualization type", "heading", "level=3");
		add_member_control(this, "complete kits", vis_type, "dropdown", "enums='hide,sphere,mesh,both'");
		add_member_control(this, "hmd", hmd_vis_type, "dropdown", "enums='hide,sphere,mesh,both'");
		add_member_control(this, "controller", controller_vis_type, "dropdown", "enums='hide,sphere,mesh,both'");
		add_member_control(this, "tracker", tracker_vis_type, "dropdown", "enums='hide,sphere,mesh,both'");
		add_member_control(this, "base", base_vis_type, "dropdown", "enums='hide,sphere,mesh,both'");

		if (begin_tree_node("sphere styles", srs, false, "level=3")) {
			align("\a");
			add_gui("sphere style", srs);
			align("\b");
			end_tree_node(srs);
		}
		align("\b");
		end_tree_node(separate_view);
	}
	if (begin_tree_node("VR events", event_flags, false, "level=2")) {
		align("\a");
		add_member_control(this, "pose_query", pose_query, "value_slider", "min=0;max=2;ticks=true");

		add_member_control(this, "debug_vr_events", debug_vr_events, "check");
		add_gui("event_flags", event_flags, "bit_field_control", "enums='dev=1,sta=2,key=4,1ax=8,2ax=16,1_k=32,2_k=64,pos=128';gui_type='toggle';options='w=30';align=''");
		align("\n\b");
		end_tree_node(event_flags);
	}
	stereo_view_interactor::create_gui();
}

/// you must overload this for gui creation
bool vr_view_interactor::self_reflect(cgv::reflect::reflection_handler& srh)
{
	return stereo_view_interactor::self_reflect(srh) &&
		srh.reflect_member("calibration_file_path", calibration_file_path) &&
		srh.reflect_member("vis_type", vis_type) &&
		srh.reflect_member("hmd_vis_type", hmd_vis_type) &&
		srh.reflect_member("controller_vis_type", controller_vis_type) &&
		srh.reflect_member("tracker_vis_type", tracker_vis_type) &&
		srh.reflect_member("base_vis_type", base_vis_type) &&
		srh.reflect_member("hmd_scale", mesh_scales[0]) &&
		srh.reflect_member("controller_scale", mesh_scales[1]) &&
		srh.reflect_member("tracker_scale", mesh_scales[2]) &&
		srh.reflect_member("base_scale", mesh_scales[3]) &&
		srh.reflect_member("hmd_mesh_file_name", hmd_mesh_file_name) &&
		srh.reflect_member("controller_mesh_file_name", controller_mesh_file_name) &&
		srh.reflect_member("tracker_mesh_file_name", tracker_mesh_file_name) &&
		srh.reflect_member("base_mesh_file_name", base_mesh_file_name) &&
		srh.reflect_member("separate_view", separate_view) &&
		srh.reflect_member("blit_vr_views", blit_vr_views) &&
		srh.reflect_member("blit_width", blit_width) &&
		srh.reflect_member("blit_aspect_scale", blit_aspect_scale) &&
		srh.reflect_member("none_separate_view", none_separate_view) &&
		srh.reflect_member("tracking_rotation", tracking_rotation) &&
		srh.reflect_member("tracking_rotation_origin_x", tracking_rotation_origin[0]) &&
		srh.reflect_member("tracking_rotation_origin_y", tracking_rotation_origin[1]) &&
		srh.reflect_member("tracking_rotation_origin_z", tracking_rotation_origin[2]) &&
		srh.reflect_member("tracking_origin_x", tracking_origin[0])&&
		srh.reflect_member("tracking_origin_y", tracking_origin[1])&&
		srh.reflect_member("tracking_origin_z", tracking_origin[2]);
}

#ifndef NO_VR_VIEW_INTERACTOR

#include <cgv/base/register.h>

/// register a newly created cube with the name "cube1" as constructor argument
cgv::base::object_registration_1<vr_view_interactor,const char*> 
 vr_interactor_reg("vr interactor", "registration of vr interactor");

// make sure shaders are embedded for single executable builds
#ifdef REGISTER_SHADER_FILES
#include <crg_vr_view_shader_inc.h>
#endif

#endif
