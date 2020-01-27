#include <cgv/base/base.h>
#include "vr_wall.h"
#include <vr/vr_driver.h>
#include <cg_vr/vr_server.h>
#include <libs/cgv_reflect_types/math/quaternion.h>
#include <cgv/gui/application.h>
#include <cgv/math/ftransform.h>

///
namespace vr {

	/// construct vr wall kit by attaching to another vr kit
	vr_wall::vr_wall() : cgv::base::node("wall")
	{
		fst_context = 0;
		vr_wall_kit_index = -1;
		vr_wall_hmd_index = -1;
		wall_kit_ptr = 0;
		screen_orientation = quat(1, 0, 0, 0);		
		window_width = 640;
		window_height = 480;
		window_x = 0;
		window_y = 0;
		cgv::signal::connect(cgv::gui::ref_vr_server().on_device_change, this, &vr_wall::on_device_change);
		kit_enum_definition = "enums='none=-1";
	}
	///
	void vr_wall::on_device_change(void* device_handle, bool attach)
	{
		kit_enum_definition = "enums='none=-1";
		std::vector<void*> kits = vr::scan_vr_kits();
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
		if (find_control(vr_wall_kit_index))
			find_control(vr_wall_kit_index)->multi_set(kit_enum_definition);
	}
	/// destruct window here
	vr_wall::~vr_wall()
	{
		cgv::base::unregister_object(window);
		cgv::gui::application::remove_window(window);
	}
	///
	std::string vr_wall::get_type_name() const 
	{
		return "vr_wall"; 
	}
	///
	void vr_wall::create_wall_window()
	{
		window = cgv::gui::application::create_window(2 * window_width, window_height, "wall_window", "viewer");
//		cgv::base::register_object(window);
		window->set("menu", false);
		window->set("gui", false);
		window->set("w", 2 * window_width);
		window->set("h", window_height);
		window->set("x", window_x);
		window->set("y", window_y);
		window->show();
		window->register_object(this, "parents=''");
	}
	///
	bool vr_wall::self_reflect(cgv::reflect::reflection_handler& srh)
	{
		return
			srh.reflect_member("vr_wall_kit_index", vr_wall_kit_index) &&
			srh.reflect_member("vr_wall_hmd_index", vr_wall_hmd_index) &&
			srh.reflect_member("screen_orientation", screen_orientation) &&
			srh.reflect_member("creation_width", window_width) &&
			srh.reflect_member("creation_width", window_height) &&
			srh.reflect_member("creation_width", window_x) &&
			srh.reflect_member("creation_width", window_y);
	}
	/// you must overload this for gui creation
	void vr_wall::create_gui()
	{
		add_decorator("VR wall", "heading");
		add_member_control(this, "vr_wall_kit", (cgv::type::DummyEnum&)vr_wall_kit_index, "dropdown", kit_enum_definition);
		add_member_control(this, "vr_wall_hmd_index", vr_wall_hmd_index, "value_slider", "min=-1;max=3;ticks=true");
		if (begin_tree_node("window creation parameters", window_width, true, "level=3")) {
			align("\a");
			add_member_control(this, "width", window_width, "value_slider", "min=320;max=3920;ticks=true;log=true");
			add_member_control(this, "height", window_height, "value_slider", "min=240;max=2160;ticks=true;log=true");
			add_member_control(this, "x", window_x, "value_slider", "min=0;max=3920;ticks=true;log=true");
			add_member_control(this, "y", window_y, "value_slider", "min=0;max=2160;ticks=true;log=true");
			align("\b");
			end_tree_node(window_width);
		}
		if (vr_wall_kit_index != -1 && wall_kit_ptr != 0) {
			add_decorator("screen", "heading", "level=3");
			add_view("width", wall_kit_ptr->width, "", "w=60", " ");
			add_view("height", wall_kit_ptr->height, "" "w=60", " ");
			add_view("multi", wall_kit_ptr->nr_multi_samples, "" "w=30");
			add_member_control(this, "pixel_size", wall_kit_ptr->pixel_size, "value_slider", "min=0.0001;max=0.01;ticks=true;log=true;step=0.00001");
			add_gui("screen_center", wall_kit_ptr->screen_center_world, "", "options='min=-3;max=3;ticks=true'");
			add_gui("screen_orientation", screen_orientation, "direction", "options='min=-1;max=1;ticks=true'");
			add_decorator("head", "heading", "level=3");
			add_member_control(this, "eye_separation", wall_kit_ptr->eye_separation, "value_slider", "min=0.01;max=0.12;ticks=true;step=0.001");
			add_gui("eye_center_tracker", wall_kit_ptr->eye_center_tracker, "", "options='min=-0.2;max=0.2;step=0.001;ticks=true'");
			add_gui("eye_separation_dir_tracker", wall_kit_ptr->eye_separation_dir_tracker, "direction", "options='min=-1;max=1;ticks=true'");
		}

	}
	///
	void vr_wall::on_set(void* member_ptr)
	{
		if (member_ptr == &vr_wall_kit_index) {
			if (vr_wall_kit_index >= 0) {
				std::vector<void*> kits = vr::scan_vr_kits();
				if (vr_wall_kit_index < (int)kits.size()) {
					if (wall_kit_ptr == 0) {
						wall_kit_ptr = new vr::vr_wall_kit(vr_wall_kit_index, window_width, window_height, "vr_wall_kit");
						connect_copy(wall_kit_ptr->on_submit_frame, cgv::signal::rebind(static_cast<drawable*>(this), &drawable::post_redraw));
					}
					else
						wall_kit_ptr->attach(vr_wall_kit_index);

					if (!wall_kit_ptr->is_attached())
						vr_wall_kit_index = -1;
					else
						screen_orientation = quat(wall_kit_ptr->get_screen_orientation());

					if (window.empty())
						create_wall_window();

					post_recreate_gui();
				}
			}
		}
		if (member_ptr >= &screen_orientation && member_ptr < &screen_orientation + 1) {
			if (wall_kit_ptr)
				wall_kit_ptr->set_screen_orientation(screen_orientation);
		}
		if (member_ptr == &vr_wall_hmd_index) {
			if (wall_kit_ptr)
				wall_kit_ptr->hmd_trackable_index = vr_wall_hmd_index;
		}
	}
	///
	bool vr_wall::init(cgv::render::context& ctx)
	{
		if (fst_context == 0)
			fst_context = &ctx;
		else {
			ctx.set_bg_clr_idx(4);
			if (!wall_kit_ptr->fbos_initialized())
				if (wall_kit_ptr->init_fbos())
					std::cout << "initialized fbos of wall kit in context " << (void*)&ctx << std::endl;
		}
		return true;
	}
	void vr_wall::clear(cgv::render::context& ctx)
	{
		if (wall_kit_ptr)
			delete wall_kit_ptr;
		wall_kit_ptr = 0;
	}
	///
	void vr_wall::draw(cgv::render::context& ctx)
	{
		if (window.empty() || wall_kit_ptr == 0)
			return;
		int width = window->get<int>("w");
		int height = window->get<int>("h");
		wall_kit_ptr->blit_fbo(0, 0, 0, width / 2, height);
		wall_kit_ptr->blit_fbo(1, width / 2, 0, width / 2, height);
		/*
		if (show_vr_kits_as_spheres && wall_kit_ptr != 0 && vr_wall_kit_index != -1) {
			vr::vr_kit_state* state_ptr = &kit_states[vr_wall_kit_index];
			vec4 s_l(wall_kit_ptr->get_eye_position_world(0, reinterpret_cast<const mat34&>(*state_ptr->hmd.pose)), 0.012f);
			vec4 s_r(wall_kit_ptr->get_eye_position_world(1, reinterpret_cast<const mat34&>(*state_ptr->hmd.pose)), 0.012f);
			if (wall_kit_ptr != rendered_kit_ptr || rendered_eye != 0) {
				spheres.push_back(s_l);
				sphere_colors.push_back(rgb(1, 0, 0));
			}
			if (wall_kit_ptr != rendered_kit_ptr || rendered_eye != 1) {
				spheres.push_back(s_r);
				sphere_colors.push_back(rgb(0, 0, 1));
			}
		}
		*/
	}
}

#include <cgv/base/register.h>

/// register a newly created cube with the name "cube1" as constructor argument
cgv::base::object_registration<vr::vr_wall> wall_reg("no options");

