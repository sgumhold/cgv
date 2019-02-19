#include "vr_view_interactor.h"
#include <cgv/render/attribute_array_binding.h>
#include <cgv/render/shader_program.h>
#include <cg_vr/vr_server.h>
#include <cgv/gui/trigger.h>
#include <cgv/signal/rebind.h>
#include <iostream>
#include <sstream>
#include <vr/vr_kit.h>
#include <vr/vr_driver.h>

///
vr_view_interactor::vr_view_interactor(const char* name) : stereo_view_interactor(name),
	fence_color1(0,0,1), fence_color2(1,1,0)
{
	debug_vr_events = false;

	fence_frequency = 5;
	fence_line_width = 3;
	show_vr_kits = true;
	show_action_zone = false;
	current_vr_handle = 0;
	current_vr_handle_index = 0;
	kit_enum_definition = "enums='none=0'";

	brs.map_color_to_material = cgv::render::MS_FRONT_AND_BACK;
	srs.map_color_to_material = cgv::render::MS_FRONT_AND_BACK;

	cgv::signal::connect(cgv::gui::ref_vr_server().on_device_change, this, &vr_view_interactor::on_device_change);
	cgv::signal::connect(cgv::gui::ref_vr_server().on_status_change, this, &vr_view_interactor::on_status_change);
	cgv::gui::connect_vr_server();
}

///
void vr_view_interactor::on_status_change(void* device_handle, int controller_index, vr::VRStatus old_status, vr::VRStatus new_status)
{
	post_redraw();
	if (debug_vr_events)
		std::cout << "on status change(" << device_handle << ","
		<< controller_index << "," << vr::get_status_string(old_status)
		<< "," << vr::get_status_string(new_status) << ")" << std::endl;
}

///
void vr_view_interactor::on_device_change(void* device_handle, bool attach)
{
	if (attach)
		new_kits.push_back(device_handle);
	else
		old_kits.push_back(device_handle);
	post_redraw();
	if (debug_vr_events)
		std::cout << "on device change(" << device_handle << ","
		<< (attach?"attach":"detach") << ")" << std::endl;
}

/// return the type name 
std::string vr_view_interactor::get_type_name() const
{
	return "vr_view_interactor";
}
/// 
void vr_view_interactor::on_set(void* member_ptr)
{
	if (member_ptr == &current_vr_handle_index) {
		if (current_vr_handle_index == 0)
			current_vr_handle = 0;
		else
			if (current_vr_handle_index - 1 < int(kits.size()))
				current_vr_handle = kits[current_vr_handle_index - 1];
	}
}

/// overload to stream help information to the given output stream
void vr_view_interactor::stream_help(std::ostream& os)
{
	stereo_view_interactor::stream_help(os);
}

/// overload to show the content of this object
void vr_view_interactor::stream_stats(std::ostream& os)
{
	stereo_view_interactor::stream_stats(os);
}

bool vr_view_interactor::init(cgv::render::context& ctx)
{
	sr.set_render_style(srs);
	if (!sr.init(ctx)) {
		exit(0);
	}
	return stereo_view_interactor::init(ctx);
}

/// overload and implement this method to handle events
bool vr_view_interactor::handle(cgv::gui::event& e)
{
	if (debug_vr_events) {
		if (e.get_kind() == cgv::gui::EID_VR || ((e.get_flags() & cgv::gui::EF_VR) != 0)) {
			e.stream_out(std::cout);
			std::cout << std::endl;
		}
	}
	return stereo_view_interactor::handle(e);
}


/// this method is called in one pass over all drawables after drawing
void vr_view_interactor::finish_frame(cgv::render::context& ctx)
{
	stereo_view_interactor::finish_frame(ctx);
}


/// this method is called in one pass over all drawables before the draw method
void vr_view_interactor::init_frame(cgv::render::context& ctx)
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
					std::cout << "initialized fbos of " << kit_ptr->get_name() << std::endl;
					if (current_vr_handle == 0) 
						current_vr_handle = new_kits.back();
				}
			kits.push_back(new_kits.back());
			update_kits = true;
		}
		new_kits.pop_back();
	}
	// 
	if (update_kits) {
		kit_enum_definition = "enums='none=0";
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
			current_vr_handle_index = 0;
		else {
			for (unsigned i = 0; i < kits.size(); ++i)
				if (kits[i] == current_vr_handle) {
					current_vr_handle_index = i + 1;
					break;
				}
		}
		update_member(&current_vr_handle_index);
	}
	stereo_view_interactor::init_frame(ctx);
}

/// 
void vr_view_interactor::draw(cgv::render::context& ctx)
{
	double time = cgv::gui::trigger::get_current_time();
	if (show_vr_kits) {
		std::vector<vec4> spheres;
		std::vector<rgb> sphere_colors;
		for (auto handle : kits) {
			vr::vr_kit* kit_ptr = vr::get_vr_kit(handle);
			if (!kit_ptr)
				continue;
			vr::vr_kit_state state;
			if (!kit_ptr->query_state(state, 1))
				continue;
			float left_eye_to_head[12];
			float right_eye_to_head[12];
			kit_ptr->put_eye_to_head_matrix(0, left_eye_to_head);
			kit_ptr->put_eye_to_head_matrix(1, right_eye_to_head);
			const mat3& R_w_h = reinterpret_cast<const mat3&>(state.hmd.pose[0]);
			const vec3& p_w_h = reinterpret_cast<const vec3&>(state.hmd.pose[9]);

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

			for (unsigned i = 0; i < 2; ++i) {
				const mat3& R_ci = reinterpret_cast<const mat3&>(state.controller[i].pose[0]);
				const vec3& p_ci = reinterpret_cast<const vec3&>(state.controller[i].pose[9]);
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
		}
		if (!spheres.empty()) {
			sr.set_sphere_array(ctx, spheres);
			sr.set_color_array(ctx, sphere_colors);
			sr.validate_and_enable(ctx);
			glDrawArrays(GL_POINTS, 0, GLsizei(spheres.size()));
			sr.disable(ctx);
		}
	}
	if (show_action_zone && current_vr_handle) {
		vr::vr_kit* kit_ptr = vr::get_vr_kit(current_vr_handle);
		if (kit_ptr) {
			const vr::vr_driver* driver_ptr = kit_ptr->get_driver();
			if (driver_ptr) {
				float h = driver_ptr->get_action_zone_height();
				vec3 up_dir;
				driver_ptr->put_up_direction(&up_dir[0]);
				std::vector<float> boundary;
				driver_ptr->put_action_zone_bounary(boundary);
				unsigned n = boundary.size() / 3;
				std::vector<vec3> G;
				G.resize(5 * n);
				unsigned i;
				for (i = 0; i < 5; ++i) {
					for (unsigned j = 0; j < n; ++j) {
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
					glDrawArrays(GL_LINE_LOOP, i*n, n);
				prog.disable(ctx);
				cgv::render::attribute_array_binding::disable_global_array(ctx, pos_idx);
				glDisable(GL_LINE_STIPPLE);
				glLineWidth(1.0f);
				post_redraw();
			}
		}
	}
	stereo_view_interactor::draw(ctx);
}

/// you must overload this for gui creation
void vr_view_interactor::create_gui()
{
	add_member_control(this, "debug_vr_events", debug_vr_events, "check");
	add_member_control(this, "show_vr_kits", show_vr_kits, "check");
	add_member_control(this, "show_action_zone", show_action_zone, "check");
	add_member_control(this, "current vr kit", (cgv::type::DummyEnum&)current_vr_handle_index, "dropdown", kit_enum_definition);
	if (begin_tree_node("render styles", show_vr_kits)) {
		align("\a");
		add_member_control(this, "fence_color1", fence_color1);
		add_member_control(this, "fence_color2", fence_color2);
		add_member_control(this, "fence_line_width", fence_line_width, "value_slider", "min=1;max=20;ticks=true;log=true");
		add_member_control(this, "fence_frequency", fence_frequency, "value_slider", "min=0.1;max=10;ticks=true;log=true");

		if (begin_tree_node("box styles", brs)) {
			align("\a");
			add_gui("box style", brs);
			align("\b");
			end_tree_node(brs);
		}
		if (begin_tree_node("sphere styles", srs)) {
			align("\a");
			add_gui("sphere style", srs);
			align("\b");
			end_tree_node(srs);
		}
		align("\b");
		end_tree_node(show_vr_kits);
	}
	stereo_view_interactor::create_gui();
}

/// you must overload this for gui creation
bool vr_view_interactor::self_reflect(cgv::reflect::reflection_handler& srh)
{
	return stereo_view_interactor::self_reflect(srh);
}


#include <cgv/base/register.h>

/// register a newly created cube with the name "cube1" as constructor argument
extern cgv::base::object_registration_1<vr_view_interactor,const char*> 
 obj1("vr interactor", "registration of vr interactor");

