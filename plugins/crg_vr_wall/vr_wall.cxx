#include <cgv/base/base.h>
#include "vr_wall.h"
#include <cgv/math/pose.h>
#include <vr/vr_driver.h>
#include <cgv/media/image/image_reader.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/gui/key_event.h>
#include <libs/cgv_reflect_types/math/quaternion.h>
#include <cgv/gui/application.h>
#include <cgv/math/ftransform.h>
#include <random>

///
namespace vr {
	cgv::gui::window_ptr vr_wall::create_wall_window(cgv::data::ref_ptr<cgv::render::callback_drawable>& _cbd_ptr, 
		const std::string& name, int x, int y, int width, int height, int fullscr, bool is_right)
	{
		cgv::gui::window_ptr W = cgv::gui::application::create_window(width, height, name, "viewer");
		W->set_name(name);
		W->set("menu", false);
		W->set("gui", false);
		W->set("w", width);
		W->set("h", height);
		W->set("x", x);
		W->set("y", y);
		W->show();
		if (fullscr > 0) {
			std::string state("fullscreen(");
			bool first = true;
			int flag = 1;
			for (int i = 1; i < 5; ++i) {
				if ((fullscr & flag) != 0) {
					if (first)
						first = false;
					else
						state += ",";
					state += cgv::utils::to_string(i);
				}
				flag *= 2;
			}
			state += ")";
			W->set("state", state);
		}
		// create callback_drawable and connect its signals to corresponding methods of vr_wall
		std::string drawable_name = "window drawable";
		if (stereo_window_mode == SWM_TWO)
			drawable_name = std::string(is_right ? "right " : "left ") + name;
		_cbd_ptr = new cgv::render::callback_drawable(drawable_name);
		if (is_right)
			connect(_cbd_ptr->init_callback, this, &vr_wall::init_cbd1);
		else
			connect(_cbd_ptr->init_callback, this, &vr_wall::init_cbd0);
		connect_copy(_cbd_ptr->clear_callback, cgv::signal::rebind(this, &vr_wall::clear_cbd, cgv::signal::_1, cgv::signal::_c(is_right)));
		connect_copy(_cbd_ptr->draw_callback, cgv::signal::rebind(this, &vr_wall::draw_cbd, cgv::signal::_1, cgv::signal::_c(is_right)));
		connect_copy(_cbd_ptr->finish_frame_callback, cgv::signal::rebind(this, &vr_wall::finish_frame_cbd, cgv::signal::_1, cgv::signal::_c(is_right)));
		// register callback drawable
		W->register_object(_cbd_ptr, "");
		return W;
	}
	/// helper function to create the window for the wall display
	void vr_wall::create_wall_windows()
	{
		switch (stereo_window_mode) {
		case SWM_SINGLE:
			window = create_wall_window(cbd_ptr, "wall window", window_x, window_y, window_width, window_height, fullscreen);
			break;
		case SWM_DOUBLE:
			window = create_wall_window(cbd_ptr, "wall window", window_x, window_y, 2 * window_width, window_height, fullscreen);
			break;
		case SWM_TWO:
			window = create_wall_window(cbd_ptr, "left wall window", window_x, window_y, window_width, window_height, fullscreen);
			right_window = create_wall_window(right_cbd_ptr, "right wall window", window_x, window_y, window_width, window_height, right_fullscreen, true);
			break;
		}
		generate_screen_calib_points();
		post_recreate_gui();
	}

	void vr_wall::generate_screen_calib_points()
	{
		float aspect = (float)window_width / window_height;
		calib_points_screen.push_back(vec3(-0.9f * aspect, 0.0f, 0.0));
		calib_points_screen.push_back(vec3( 0.9f * aspect, 0.0f, 0.0));
		calib_points_screen.push_back(vec3(0.0f, -0.5f, 0.0f));
		calib_points_screen.push_back(vec3(0.0f,  0.5f, 0.0f));
		calib_points_world = calib_points_screen;
	}
	/// update screen calibration
	void vr_wall::on_update_screen_calibration()
	{
		if (!wall_kit_ptr)
			return;
		mat34& screen_pose = wall_kit_ptr->screen_pose;
		vec3& x = reinterpret_cast<vec3&>(screen_pose(0, 0));
		vec3& y = reinterpret_cast<vec3&>(screen_pose(0, 1));
		vec3& z = reinterpret_cast<vec3&>(screen_pose(0, 2));
		screen_pose.set_col(3, screen_center);
		x = screen_x;
		double l_x = x.normalize();
		y = screen_y;
		double l_y = y.normalize();
		z = cross(x, y);
		screen_orientation = quat(wall_kit_ptr->get_screen_orientation());
		std::cout << "update:\n" << screen_pose << std::endl;
		wall_kit_ptr->pixel_size[0] = (float)(2 * l_x / wall_kit_ptr->width);
		wall_kit_ptr->pixel_size[1] = (float)(2 * l_y / wall_kit_ptr->height);
		update_member(&wall_kit_ptr->pixel_size[0]);
		update_member(&wall_kit_ptr->pixel_size[1]);
		update_member(&screen_orientation[0]);
		update_member(&screen_orientation[1]);
		update_member(&screen_orientation[2]);
		update_member(&screen_orientation[3]);
		if (!screen_calibration_file_name.empty())
			write_screen_calibration(screen_calibration_file_name);
	}
	/// construct vr wall kit by attaching to another vr kit
	vr_wall::vr_wall() : cgv::base::node("vr_wall")
	{
		blit_fbo = -1;
		blit_tex[0] = blit_tex[1] = -1;
		stereo_shader_mode = SSM_SIDE_BY_SIDE;
		stereo_window_mode = SWM_DOUBLE;
		fullscreen = 0;
		right_fullscreen = 0;
		wall_kit_ptr = 0;
		peek_point = vec3(0, -0.0755634f, -0.0385344f);
		ctrl_down_dir = normalize(vec3(0.0f, -0.53f, 0.85f));
		ctrl_forward_dir = normalize(vec3(0.0f, -0.85f, -0.53f));
		IPD = 0.067f;
		ctrl_upside_down_index = -1;
		eye_position_tracker[0] = vec3(-0.03f, -0.04f, 0.0f);
		eye_position_tracker[1] = vec3(0.03f, -0.04f, 0.0f);
		eye_calibrated[0] = eye_calibrated[1] = false;
		vr_wall_kit_index = -1;
		vr_wall_hmd_index = -1;
		screen_orientation = quat(1, 0, 0, 0);
		screen_center = vec3(0, 0.8f, 1);
		screen_x = vec3(-0.235f, 0, 0);
		screen_y = vec3(0, 0.135f, 0);
		window_width = 1920;
		window_height = 1080;
		window_x = 0;
		window_y = 0;
		calib_index = -1;
		auto_eyes_calib = true;
		prs.halo_color = rgba(0, 0, 0, 0.9f);
		prs.halo_width_in_pixel = -2.0f;
		prs.point_size = 15.0f;
		prs.measure_point_size_in_pixel = true;
		cgv::signal::connect(cgv::gui::ref_vr_server().on_device_change, this, &vr_wall::on_device_change);
		kit_enum_definition = "enums='none=-1";
		hmd_pose.identity();

		wall_state = WS_HMD;
		on_set(&wall_state);
	}
	///
	vr_wall::~vr_wall()
	{
		if (window)
			cgv::gui::application::remove_window(window);
		if (right_window)
			cgv::gui::application::remove_window(right_window);
		if (wall_kit_ptr) {
			delete wall_kit_ptr;
			wall_kit_ptr = 0;
		}
	}
	///
	void vr_wall::on_device_change(void* handle, bool attach)
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
	///
	std::string vr_wall::get_type_name() const
	{
		return "vr_wall";
	}
	///
	bool vr_wall::self_reflect(cgv::reflect::reflection_handler& srh)
	{
		return
			srh.reflect_member("screen_calibration_file_name", screen_calibration_file_name) &&
			srh.reflect_member("vr_wall_kit_index", vr_wall_kit_index) &&
			srh.reflect_member("auto_eyes_calib", auto_eyes_calib) &&
			srh.reflect_member("vr_wall_hmd_index", vr_wall_hmd_index) &&
			srh.reflect_member("screen_orientation", screen_orientation) &&
			srh.reflect_member("screen_center", screen_center) &&
			srh.reflect_member("screen_x", screen_x) &&
			srh.reflect_member("screen_y", screen_y) &&
			srh.reflect_member("IPD", IPD) &&
			srh.reflect_member("wall_state", (int&)wall_state) &&
			srh.reflect_member("peek_point_x", peek_point[0]) &&
			srh.reflect_member("peek_point_y", peek_point[1]) &&
			srh.reflect_member("peek_point_z", peek_point[2]) &&
			srh.reflect_member("fullscreen", fullscreen) &&
			srh.reflect_member("left_fullscreen", fullscreen) &&
			srh.reflect_member("right_fullscreen", right_fullscreen) &&
			srh.reflect_member("stereo_shader_mode", (int&)stereo_shader_mode) &&
			srh.reflect_member("stereo_window_mode", (int&)stereo_window_mode) &&
			srh.reflect_member("creation_width", window_width) &&
			srh.reflect_member("creation_height", window_height) &&
			srh.reflect_member("creation_x", window_x) &&
			srh.reflect_member("creation_y", window_y);
	}
	///
	bool vr_wall::read_screen_calibration(const std::string& file_name)
	{
		std::ifstream is(file_name);
		if (is.fail())
			return false;
		int w, h;
		vec2 ps;
		vec3 ctr;
		quat ori;
		is >> w >> h >> ps >> ctr >> ori;
		if (is.fail())
			return false;
		wall_kit_ptr->set_screen_orientation(ori);
		wall_kit_ptr->screen_pose.set_col(3, ctr);
		wall_kit_ptr->pixel_size = ps;
		wall_kit_ptr->width = w;
		wall_kit_ptr->height = h;
		screen_center = ctr;
		float l_x = 0.5f * ps[0] * w;
		float l_y = 0.5f * ps[1] * h;
		screen_x = l_x*wall_kit_ptr->screen_pose.col(0);
		screen_y = l_y*wall_kit_ptr->screen_pose.col(1);
		screen_orientation = ori;
		update_all_members();
		return true;
	}
	///
	bool vr_wall::write_screen_calibration(const std::string& file_name) const
	{
		std::ofstream os(file_name);
		if (os.fail())
			return false;
		os << wall_kit_ptr->width << " " << wall_kit_ptr->height << " " << wall_kit_ptr->pixel_size << " ";
		mat34& screen_pose = wall_kit_ptr->screen_pose;
		os << screen_pose.col(3) << " " << quat(wall_kit_ptr->get_screen_orientation()) << std::endl;
		os.close();
		return true;
	}
	///
	void vr_wall::on_set(void* member_ptr)
	{
		if (member_ptr == &screen_calibration_file_name) {
			read_screen_calibration(screen_calibration_file_name);
		}
		if (member_ptr == &wall_state) {
			switch (wall_state) {
			case WS_SCREEN_CALIB:
			case WS_EYES_CALIB:
				calib_index = 0;
				update_member(&calib_index);
				if (wall_kit_ptr)
					wall_kit_ptr->in_calibration = true;
				cgv::gui::ref_vr_server().grab_focus(cgv::gui::VRF_GRAB_EXCLUSIVE, this);
				break;
			case WS_HMD:
				if (wall_kit_ptr)
					wall_kit_ptr->in_calibration = false;
				cgv::gui::ref_vr_server().release_focus(this);
				break;
			}
		}
		else if (member_ptr == &vr_wall_kit_index) {
			if (vr_wall_kit_index >= 0) {
				std::vector<void*> kits = vr::scan_vr_kits();
				if (vr_wall_kit_index < (int)kits.size()) {
					if (wall_kit_ptr == 0) {
						wall_kit_ptr = new vr::vr_wall_kit(vr_wall_kit_index, window_width, window_height, "vr_wall_kit");
						if (wall_state != WS_HMD)
							wall_kit_ptr->in_calibration = true;
						on_update_screen_calibration();
						connect_copy(wall_kit_ptr->on_submit_frame, cgv::signal::rebind(this, &vr_wall::post_redraw_all));
					}
					else
						wall_kit_ptr->attach(vr_wall_kit_index);

					if (!wall_kit_ptr->is_attached())
						vr_wall_kit_index = -1;
					else
						screen_orientation = quat(wall_kit_ptr->get_screen_orientation());

					if (window.empty())
						create_wall_windows();

					post_recreate_gui();
				}
			}
			else {
				if (wall_kit_ptr->is_attached()) {
					wall_kit_ptr->detach();
					delete wall_kit_ptr;
					wall_kit_ptr = 0;
					window->hide();
					cgv::gui::application::remove_window(window);
					window.clear();
					if (right_window) {
						right_window->hide();
						cgv::gui::application::remove_window(window);
						right_window.clear();
					}
					post_recreate_gui();
				}
			}
		}
		else if (member_ptr == &stereo_window_mode && !window.empty()) {
			if (!window.empty()) {
				window->hide();
				cgv::gui::application::remove_window(window);
				window.clear();
			}
			if (!right_window.empty()) {
				right_window->hide();
				cgv::gui::application::remove_window(right_window);
				right_window.clear();
			}
			create_wall_windows();
		}
		else if (
			(member_ptr >= &screen_center && member_ptr < &screen_center + 1) ||
			(member_ptr >= &screen_x && member_ptr < &screen_x + 1) ||
			(member_ptr >= &screen_y && member_ptr < &screen_y + 1)) {
			on_update_screen_calibration();
		}
		else if (member_ptr >= &screen_orientation && member_ptr < &screen_orientation + 1) {
			if (wall_kit_ptr)
				wall_kit_ptr->set_screen_orientation(screen_orientation);
		}
		else if (member_ptr == &vr_wall_hmd_index) {
			if (wall_kit_ptr)
				wall_kit_ptr->hmd_trackable_index = vr_wall_hmd_index;
		}
		update_member(member_ptr);
		post_redraw_all();
	}

	void vr_wall::post_redraw_all()
	{
		post_redraw();
		if (cbd_ptr)
			cbd_ptr->post_redraw();
		if (right_cbd_ptr)
			right_cbd_ptr->post_redraw();
	}

	/// you must overload this for gui creation
	void vr_wall::create_gui()
	{
		add_decorator("VR wall", "heading");

		if (begin_tree_node("window creation parameters", window_width, true, "level=2")) {
			align("\a");
			add_member_control(this, "width", window_width, "value_slider", "min=320;max=3920;ticks=true;log=true");
			add_member_control(this, "height", window_height, "value_slider", "min=240;max=2160;ticks=true;log=true");
			add_member_control(this, "x", window_x, "value_slider", "min=0;max=3920;ticks=true;log=true");
			add_member_control(this, "y", window_y, "value_slider", "min=0;max=2160;ticks=true;log=true");
			add_member_control(this, "fullscreen", (cgv::type::DummyEnum&)fullscreen, "dropdown", "enums='no fullscreen,monitor 1, monitor 2,monitor 1+2, monitor 3, monitor 1+3, monitor 2+3, monitor 1+2+3'");
			add_member_control(this, "stereo_window_mode", stereo_window_mode, "dropdown", "enums='single,double,two'");
			align("\b");
			end_tree_node(window_width);
		}
		add_member_control(this, "vr_wall_kit", (cgv::type::DummyEnum&)vr_wall_kit_index, "dropdown", kit_enum_definition);
		add_member_control(this, "vr_wall_hmd_index", vr_wall_hmd_index, "value_slider", "min=-1;max=3;ticks=true");
		add_member_control(this, "stereo_shader_mode", stereo_shader_mode, "dropdown", "enums='left only,right only,side by side,top bottom,column interleaved,row interleaved,red|cyan anaglyph,color anaglyph,half-color anaglyph,Dubois anaglyph'");

		if (calib_points_screen.size() >= 4 && begin_tree_node("screen calibration", screen_center, false, "level=2")) {
			align("\a");
			add_gui("cp0", calib_points_screen[0], "", "options='min=-2;max=2;step=0.0001;ticks=true'");
			add_gui("cp1", calib_points_screen[1], "", "options='min=-2;max=2;step=0.0001;ticks=true'");
			add_gui("cp2", calib_points_screen[2], "", "options='min=-2;max=2;step=0.0001;ticks=true'");
			add_gui("cp3", calib_points_screen[3], "", "options='min=-2;max=2;step=0.0001;ticks=true'");

			add_gui("peek_point", peek_point, "", "options='min=-0.1;max=0.1;step=0.0001;ticks=true'");
			if (begin_tree_node("point render style", prs, false, "level=3")) {
				align("\a");
				add_gui("point_style", prs);
				align("\b");
				end_tree_node(prs);
			}

			add_gui("screen_center", screen_center, "", "long_label=true;options='min=-3;max=3;ticks=true'");
			add_gui("screen_x", screen_x, "", "long_label=true;options='min=-3;max=3;ticks=true'");
			add_gui("screen_y", screen_y, "", "long_label=true;options='min=-3;max=3;ticks=true'");
			add_gui("screen_orientation", (vec4&)screen_orientation, "direction", "long_label=true;options='min=-1;max=1;ticks=true'");

			if (vr_wall_kit_index != -1 && wall_kit_ptr != 0) {
				add_decorator("screen", "heading", "level=2");
				add_view("width", wall_kit_ptr->width, "", "w=40", " ");
				add_view("height", wall_kit_ptr->height, "", "w=40", " ");
				add_view("multi", wall_kit_ptr->nr_multi_samples, "", "w=20");
				add_member_control(this, "pixel_size_x", wall_kit_ptr->pixel_size[0], "value_slider", "min=0.0001;max=0.01;ticks=true;log=true;step=0.00001");
				add_member_control(this, "pixel_size_y", wall_kit_ptr->pixel_size[1], "value_slider", "min=0.0001;max=0.01;ticks=true;log=true;step=0.00001");
			}
			align("\b");
			end_tree_node(screen_center);
		}

		if (begin_tree_node("eye calibration", prs, false, "level=2")) {
			align("\a");
			add_member_control(this, "auto_eyes_calib", auto_eyes_calib, "toggle");
			add_view("left eye", eye_calibrated[0], "check");
			add_gui("left eye", eye_position_tracker[0], "", "options='min=-0.2;max=0.2;step=0.001;ticks=true'");
			add_view("right eye", eye_calibrated[1], "check");
			add_gui("right eye", eye_position_tracker[1], "", "options='min=-0.2;max=0.2;step=0.001;ticks=true'");
			if (wall_kit_ptr) {
				add_decorator("head", "heading", "level=2");
				add_member_control(this, "eye_separation", wall_kit_ptr->eye_separation, "value_slider", "min=0.01;max=0.12;ticks=true;step=0.001");
				add_gui("eye_center_tracker", wall_kit_ptr->eye_center_tracker, "", "options='min=-0.2;max=0.2;step=0.001;ticks=true'");
				add_gui("eye_separation_dir_tracker", wall_kit_ptr->eye_separation_dir_tracker, "direction", "options='min=-1;max=1;ticks=true'");
			}
			align("\b");
			end_tree_node(prs);
		}

	}
	// handle screen calibration specific keys
	bool vr_wall::handle_key_event_screen_calib(cgv::gui::vr_key_event& vrke)
	{
		switch (vrke.get_key()) {
			// grip buttons
		case vr::VR_GRIP:
			calib_points_world[calib_index] = reinterpret_cast<const mat34&>(vrke.get_state().controller[vrke.get_controller_index()].pose[0])* vec4(peek_point, 1.0f);
			switch (calib_index) {
			case 0:
				on_update_screen_calibration();
				break;
			case 1:
				screen_center = 0.5f * (calib_points_world[0] + calib_points_world[1]);
				screen_x = (1.0f/1.8f)*(calib_points_world[1]- calib_points_world[0]);
				on_update_screen_calibration();
				update_member(&screen_center[0]);
				update_member(&screen_center[1]);
				update_member(&screen_center[2]);
				update_member(&screen_x[0]);
				update_member(&screen_x[1]);
				update_member(&screen_x[2]);
				break;
			case 2:
				screen_y = 2.0f * (screen_center - calib_points_world[2]);
				on_update_screen_calibration();
				update_member(&screen_y[0]);
				update_member(&screen_y[1]);
				update_member(&screen_y[2]);
				break;
			case 3:
				screen_y = calib_points_world[3] - calib_points_world[2];
				on_update_screen_calibration();
				update_member(&screen_y[0]);
				update_member(&screen_y[1]);
				update_member(&screen_y[2]);
				break;
			}
			++calib_index;
			on_set(&calib_index);
			return true;
		}
		return false;
	}
	// handle screen calibration specific keys
	bool vr_wall::handle_key_event_eyes_calib(cgv::gui::vr_key_event& vrke)
	{
		// update hmd pose
		hmd_pose = *reinterpret_cast<const mat34*>(
			(vr_wall_hmd_index == -1) ? vrke.get_state().hmd.pose :
										vrke.get_state().controller[vr_wall_hmd_index].pose);

		if (vrke.get_key() != vr::VR_GRIP)
			return false;
		int ci = vrke.get_controller_index();
		const mat34& ctrl_pose = *reinterpret_cast<const mat34*>(vrke.get_state().controller[ci].pose);
		mat34 ctrl2hmd = cgv::math::pose_concat(cgv::math::pose_inverse(hmd_pose), ctrl_pose);
		eye_position_tracker[0] = ctrl2hmd * vec4(peek_point + 0.01f * ctrl_forward_dir - vec3(0.5f * IPD, 0.0f, 0.0f), 1.0f);
		eye_position_tracker[1] = ctrl2hmd * vec4(peek_point + 0.01f * ctrl_forward_dir + vec3(0.5f * IPD, 0.0f, 0.0f), 1.0f);
		eye_calibrated[0] = eye_calibrated[1] = true;
		for (int eye=0;eye<2;++eye) {
			update_member(&eye_calibrated[eye]);
			for (int i = 0; i < 3; ++i)
				update_member(&eye_position_tracker[eye][i]);		
		}
		return true;
	}
	///
	bool vr_wall::handle(cgv::gui::event& e)
	{
		if (e.get_kind() == cgv::gui::EID_KEY) {
			auto& ke = dynamic_cast<cgv::gui::key_event&>(e);
			if (ke.get_action() != cgv::gui::KA_RELEASE) {
				if (ke.get_key() == 'W' && ke.get_modifiers() == (cgv::gui::EM_ALT + cgv::gui::EM_CTRL)) {
					if (vr_wall_kit_index == -1)
						vr_wall_kit_index = 0;
					else
						vr_wall_kit_index = -1;
					on_set(&vr_wall_kit_index);
					return true;
				}
			}
		}
		if (wall_kit_ptr == 0)
			return false;

		// in pose events just keep track of all poses
		if (e.get_kind() == cgv::gui::EID_POSE) {
			if ((e.get_flags() & cgv::gui::EF_VR) != 0) {
				auto& vrpe = dynamic_cast<cgv::gui::vr_pose_event&>(e);
				int ci = vrpe.get_trackable_index();
				if (ci >= 0 && ci < 2) {
					controller_pose[ci] = vrpe.get_pose_matrix();
					if ((controller_pose[ci] * vec4(ctrl_down_dir, 0.0f)).y() > 0.8f) {
						ctrl_upside_down_index = ci;
						if (auto_eyes_calib && wall_state != WS_EYES_CALIB) {
							wall_state = WS_EYES_CALIB;
							on_set(&wall_state);
						}
					}
					else if (ci == ctrl_upside_down_index) {
						if (auto_eyes_calib && wall_state == WS_EYES_CALIB) {
							wall_state = WS_HMD;
							on_set(&wall_state);
						}
					}
				}
				if (ci == vr_wall_hmd_index)
					hmd_pose = vrpe.get_pose_matrix();
				post_redraw_all();
			}
		}
		if (e.get_kind() != cgv::gui::EID_KEY)
			return false;
		auto& ke = dynamic_cast<cgv::gui::key_event&>(e);
		if (ke.get_action() == cgv::gui::KA_RELEASE)
			return false;
		// handle common keys
		switch (ke.get_key()) {
		case 'S':
			if (ke.get_modifiers() != (cgv::gui::EM_ALT + cgv::gui::EM_CTRL))
				return false;
			wall_state = WS_SCREEN_CALIB;
			on_set(&wall_state);
			return true;
		case 'E':
			if (ke.get_modifiers() != (cgv::gui::EM_ALT + cgv::gui::EM_CTRL))
				return false;
			wall_state = WS_EYES_CALIB;
			on_set(&wall_state);
			return true;
		case 'H':
			if (ke.get_modifiers() != (cgv::gui::EM_ALT + cgv::gui::EM_CTRL))
				return false;
			wall_state = WS_HMD;
			on_set(&wall_state);
			return true;
		case 'A':
			if (ke.get_modifiers() != (cgv::gui::EM_ALT + cgv::gui::EM_CTRL))
				return false;
			auto_eyes_calib = !auto_eyes_calib;
			on_set(&auto_eyes_calib);
			return true;
//		case vr::VR_DPAD_LEFT:
//			if (wall_state > WS_SCREEN_CALIB) {
//				wall_state = WallState(wall_state - 1);
//				on_set(&wall_state);
//			}
//			break;
		}
		// for calibration specific keys we are only interested in vr keys
		if ((ke.get_flags() & cgv::gui::EF_VR) == 0)
			return false;
		auto& vrke = dynamic_cast<cgv::gui::vr_key_event&>(ke);

		switch (wall_state) {
		case WS_SCREEN_CALIB: return handle_key_event_screen_calib(vrke);
		case WS_EYES_CALIB: return handle_key_event_eyes_calib(vrke);
		}
		return false;
	}
	///
	void vr_wall::stream_help(std::ostream& os)
	{
		os << "vr_wall:\n"
			<< "  <C+A-S|E|H> .. select mode <Screen calib|Eye calib|Hmd>, <A+C-A> toggle auto eye calib\n"
			<< "  <left|right VR Controller Grip> .. define point\n"
			<< "  Screen calib: touch green points with controller front\n"
			<< "  Eye calib: point with upside-down controller in between your eyes and press grip" << std::endl;
	}
	void vr_wall::stream_stats(std::ostream& os)
	{
		os << "vr_wall: WM=";
		switch (stereo_window_mode) {
		case SWM_SINGLE: os << "single"; break;
		case SWM_DOUBLE: os << "double"; break;
		case SWM_TWO: os << "two"; break;
		}
		os << ",kit=" << vr_wall_kit_index << ",hmd=" << vr_wall_hmd_index << ",state=";
		switch (wall_state) {
		case WS_SCREEN_CALIB: os << "screen"; break;
		case WS_EYES_CALIB: os << "eye"; break;
		case WS_HMD: os << "hmd"; break;
		}
		os << ",idx=" << calib_index << ",IPD=" << IPD << ",stereo=";
		switch (stereo_shader_mode) {
		case SSM_LEFT_ONLY: os << "L"; break;
		case SSM_RIGHT_ONLY: os << "R"; break;
		case SSM_SIDE_BY_SIDE: os << "SbS"; break;
		case SSM_TOP_BOTTOM: os << "B"; break;
		case SSM_COLUMN_INTERLEAVED: os << "CI"; break;
		case SSM_ROW_INTERLEAVED: os << "RI"; break;
		case SSM_ANAGLYPH_RED_CYAN: os << "ARC"; break;
		case SSM_ANAGLYPH_COLOR: os << "ACol"; break;
		case SSM_ANAGLYPH_HALF_COLOR: os << "AHCol"; break;
		case SSM_ANAGLYPH_DUBOID: os << "ADub"; break;
		}
		os << ",eye";
		if (auto_eyes_calib)
			os << "*";
		os << "=";
		if (eye_calibrated[0])
			os << "L";
		if (eye_calibrated[1])
			os << "R";
		os << "\n";
	}

	///
	bool vr_wall::init_cbd(cgv::render::context& ctx, bool is_right)
	{
		bool result = true;
		wall_kit_ptr->wall_context = true;
		EyeSelection es = ES_BOTH;
		if (stereo_window_mode == SWM_TWO)
			es = is_right ? ES_RIGHT : ES_LEFT;
		if (!wall_kit_ptr->fbos_initialized(es))
			if (wall_kit_ptr->init_fbos(es)) {
				std::cout << "initialized fbos of wall kit in context " << (void*)&ctx << std::endl;
				result = false;
			}
		wall_kit_ptr->wall_context = false;

		ctx.set_default_render_pass_flags((cgv::render::RenderPassFlags)(
			cgv::render::RPF_CLEAR_COLOR |
			cgv::render::RPF_DRAWABLES_INIT_FRAME |
			cgv::render::RPF_SET_CLEAR_COLOR |
			cgv::render::RPF_DRAWABLES_DRAW |
			cgv::render::RPF_DRAWABLES_FINISH_FRAME |
			cgv::render::RPF_DRAW_TEXTUAL_INFO |
			cgv::render::RPF_DRAWABLES_AFTER_FINISH |
			cgv::render::RPF_HANDLE_SCREEN_SHOT));
		ctx.set_bg_clr_idx(4);
		pr.init(ctx);
		cgv::render::ref_point_renderer(ctx, 1);
		return result;
	}
	
	void vr_wall::clear_cbd(cgv::render::context& ctx, bool is_right)
	{
		EyeSelection es = ES_BOTH;
		if (stereo_window_mode == SWM_TWO)
			es = is_right ? ES_RIGHT : ES_LEFT;
		if (wall_kit_ptr) {
			wall_kit_ptr->wall_context = true;
			wall_kit_ptr->destruct_fbos(es);
			wall_kit_ptr->wall_context = false;
		}
		cgv::render::ref_point_renderer(ctx, -1);
	}
	///
	bool vr_wall::init(cgv::render::context& ctx)
	{
		cgv::render::ref_point_renderer(ctx, 1);
		if (!stereo_prog.build_program(ctx, "stereo.glpr")) {
			std::cerr << "could not build stereo shader program" << std::endl;
			return false;
		}
		return true;
	}
	///
	void vr_wall::clear(cgv::render::context& ctx)
	{
		cgv::render::ref_point_renderer(ctx, -1);
		stereo_prog.destruct(ctx);
		if (wall_kit_ptr)
			wall_kit_ptr->destruct_fbos();
	}
	///
	void vr_wall::draw(cgv::render::context& ctx)
	{
		if (!wall_kit_ptr)
			return;

		if (wall_state == WS_EYES_CALIB) {
			std::vector<vec3> Ps;
			std::vector<float> Rs;
			std::vector<rgb> Cs;
			for (unsigned ci = 0; ci < 2; ++ci) {
				Ps.push_back(controller_pose[ci] * vec4(peek_point, 1.0f));
				Rs.push_back(0.01f);
				Cs.push_back(rgb(1.0f - ci, 0.5f, ci));

				Ps.push_back(controller_pose[ci] * vec4(peek_point + 0.01f * ctrl_forward_dir - vec3(0.5f * IPD, 0.0f, 0.0f), 1.0f));
				Rs.push_back(0.025f);
				Cs.push_back(rgb(0.75f+0.25f*(1 - ci), 0.0f, 0.25f*ci));

				Ps.push_back(controller_pose[ci] * vec4(peek_point + 0.01f * ctrl_forward_dir + vec3(0.5f * IPD, 0.0f, 0.0f), 1.0f));
				Cs.push_back(rgb(0.25f * (1 - ci), 0.0f, 0.75f + 0.25f * ci));
				Rs.push_back(0.025f);
			}
			auto& sr = cgv::render::ref_sphere_renderer(ctx);
			sr.set_position_array(ctx, Ps);
			sr.set_color_array(ctx, Cs);
			sr.set_radius_array(ctx, Rs);
			sr.render(ctx, 0, Ps.size());
		}
		// draw textured screen rectangle in  birds eye view
		if (wall_state != WS_HMD || ctx.get_render_pass() == cgv::render::RP_MAIN) {
			std::vector<vec3> P;
			std::vector<vec2> T;
			vec3 z = cross(screen_x, screen_y);
			z.normalize();
			z *= 0.001f;
			P.push_back(screen_center + z - screen_x - screen_y); T.push_back(vec2(0.0f, 0.0f));
			P.push_back(screen_center + z + screen_x - screen_y); T.push_back(vec2(1.0f, 0.0f));
			P.push_back(screen_center + z - screen_x + screen_y); T.push_back(vec2(0.0f, 1.0f));
			P.push_back(screen_center + z + screen_x + screen_y); T.push_back(vec2(1.0f, 1.0f));
			auto& prog = stereo_prog; 
			prog.enable(ctx);
			prog.set_uniform(ctx, "left_texture", 0);
			prog.set_uniform(ctx, "right_texture", 1);
			prog.set_uniform(ctx, "stereo_mode", int(stereo_shader_mode));
			ctx.set_color(rgba(1, 1, 1, 1));
			glActiveTexture(GL_TEXTURE0);
			if (wall_state == WS_HMD && ctx.get_render_pass() == cgv::render::RP_MAIN) {
				wall_kit_ptr->bind_texture(0);
				glActiveTexture(GL_TEXTURE1);
				wall_kit_ptr->bind_texture(1);
			}
			else if (blit_tex[0] != -1) {
				glBindTexture(GL_TEXTURE_2D, blit_tex[0]);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, blit_tex[1]);
			}
			glActiveTexture(GL_TEXTURE0);
			cgv::render::attribute_array_binding::set_global_attribute_array(ctx, prog.get_position_index(), P);
			cgv::render::attribute_array_binding::enable_global_array(ctx, prog.get_position_index());
			cgv::render::attribute_array_binding::set_global_attribute_array(ctx, prog.get_texcoord_index(), T);
			cgv::render::attribute_array_binding::enable_global_array(ctx, prog.get_texcoord_index());
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			cgv::render::attribute_array_binding::disable_global_array(ctx, prog.get_position_index());
			cgv::render::attribute_array_binding::disable_global_array(ctx, prog.get_texcoord_index());
			prog.disable(ctx);
		}
	}
	///
	void vr_wall::draw_cbd(cgv::render::context& ctx, bool is_right)
	{
		// check for wall kit
		if (wall_kit_ptr == 0)
			return;

		// check for window
		if (stereo_window_mode == SWM_TWO && is_right) {
			if (right_window.empty())
				return;
		}
		else {
			if (window.empty())
				return;
		}
		// determine size and aspect ratio of wall window in pixel
		int width = ctx.get_width(), height = ctx.get_height();
		double aspect = double(width) / height;
		// determine pixel offset for rendering second eye on wall window
		int x_off = 0, y_off = 0;
		if (stereo_window_mode == SWM_DOUBLE) {
			width /= 2;
			x_off = width;
		}
		// determine size and aspect ratio of window panel for single eye in wall window
		int w = width, h = height; 
		if (stereo_window_mode == SWM_SINGLE) {
			switch (stereo_shader_mode) {
			case SSM_SIDE_BY_SIDE: w /= 2; x_off = w; break;
			case SSM_TOP_BOTTOM: h /= 2; y_off = h; break;
			}
		}
		double a = double(w) / h;
		// in hmd mode blit per eye renderings into the wall window
		if (wall_state == WS_HMD) {
			if (stereo_window_mode == SWM_SINGLE) {
				ctx.set_viewport(cgv::ivec4(0, 0, width, height));
				ctx.set_projection_matrix(cgv::math::perspective4<double>(90, aspect, 0.1, 10.0));
				ctx.set_modelview_matrix(cgv::math::look_at4<double>(cgv::dvec3(0, 0, 1), cgv::dvec3(0, 0, 0), cgv::dvec3(0, 1, 0)));
				std::vector<vec3> P;
				std::vector<vec2> T;
				P.push_back(vec3((float)-aspect, -1.0f, 0.0f)); T.push_back(vec2(0.0f, 0.0f));
				P.push_back(vec3((float)aspect, -1.0f, 0.0f)); T.push_back(vec2(1.0f, 0.0f));
				P.push_back(vec3((float)-aspect, 1.0f, 0.0f)); T.push_back(vec2(0.0f, 1.0f));
				P.push_back(vec3((float)aspect, 1.0f, 0.0f)); T.push_back(vec2(1.0f, 1.0f));
				auto& prog = stereo_prog;
				prog.enable(ctx);
				prog.set_uniform(ctx, "left_texture", 0);
				prog.set_uniform(ctx, "right_texture", 1);
				prog.set_uniform(ctx, "stereo_mode", int(stereo_shader_mode));
				prog.set_uniform(ctx, "gamma", 1.0f);
				ctx.set_color(rgba(1, 1, 1, 1));
				wall_kit_ptr->wall_context = false;
				glActiveTexture(GL_TEXTURE0);
				wall_kit_ptr->bind_texture(0);
				glActiveTexture(GL_TEXTURE1);
				wall_kit_ptr->bind_texture(1);
				glActiveTexture(GL_TEXTURE0);
				cgv::render::attribute_array_binding::set_global_attribute_array(ctx, prog.get_position_index(), P);
				cgv::render::attribute_array_binding::enable_global_array(ctx, prog.get_position_index());
				cgv::render::attribute_array_binding::set_global_attribute_array(ctx, prog.get_texcoord_index(), T);
				cgv::render::attribute_array_binding::enable_global_array(ctx, prog.get_texcoord_index());
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				cgv::render::attribute_array_binding::disable_global_array(ctx, prog.get_position_index());
				cgv::render::attribute_array_binding::disable_global_array(ctx, prog.get_texcoord_index());
				prog.disable(ctx);
			}
			else if (stereo_window_mode == SWM_DOUBLE) {
				wall_kit_ptr->wall_context = true;
				wall_kit_ptr->blit_fbo(0, 0, 0, width, height);
				wall_kit_ptr->blit_fbo(1, width, 0, width, height);
				wall_kit_ptr->wall_context = false;
			}
			else {
				wall_kit_ptr->wall_context = true;
				if (!is_right)
					wall_kit_ptr->blit_fbo(0, 0, 0, width, height);
				else
					wall_kit_ptr->blit_fbo(1, 0, 0, width, height);
				wall_kit_ptr->wall_context = false;
			}
			return;
		}

		// collect to be rendered points
		std::vector<vec3> P;
		std::vector<rgb> C;
		std::vector<float> R;

		// all tracked entities
		P.push_back(wall_kit_ptr->transform_world_to_screen(pose_position(hmd_pose)));
		P.back()[2] = 0;
		C.push_back(rgb(0, 0, 0));
		R.push_back(15.0f);
		if (wall_state == WS_SCREEN_CALIB) {
			for (const auto& p : calib_points_screen) {
				P.push_back(p);
				if (&p - &calib_points_screen.front() == calib_index)
					C.push_back(rgb(0, 1, 0));
				else
					C.push_back(rgb(0.5f, 0.5f, 0.5f));
				R.push_back(15.0f);
			}
		}

		for (int i = 0; i < 2; ++i) {
			// controller peek points
			P.push_back(wall_kit_ptr->transform_world_to_screen(pose_transform_point(controller_pose[i], peek_point)));
			P.back()[2] = 0;
			C.push_back(rgb(0.8f*float(1 - i)+0.2f, 0.2f, 0.8f*float(i)+0.2f));
			R.push_back(15.0f);
			// eye points
			if (eye_calibrated[i]) {
				P.push_back(wall_kit_ptr->transform_world_to_screen(pose_transform_point(hmd_pose, eye_position_tracker[i])));
				C.push_back(rgb(float(1 - i), 0, float(i)));
				R.push_back(0.0144f / (0.6f + P.back()[2]) / wall_kit_ptr->pixel_size[0]);
				P.back()[2] = 0;
			}
		}
		// use point renderer for rendering of points
		for (int eye = 0; eye < 2; ++eye) {
			ctx.set_viewport(cgv::ivec4(eye * x_off, eye * y_off, w, h));
			ctx.set_projection_matrix(cgv::math::perspective4<double>(90, a, 0.1, 10.0));
			ctx.set_modelview_matrix(cgv::math::look_at4<double>(cgv::dvec3(0, 0, 1), cgv::dvec3(0, 0, 0), cgv::dvec3(0, 1, 0)));
			pr.set_y_view_angle(90);
			pr.set_render_style(prs);
			pr.set_position_array(ctx, P);
			pr.set_color_array(ctx, C);
			pr.set_point_size_array(ctx, R);
			pr.render(ctx, 0, P.size());
		}
	}
	///
	void vr_wall::finish_frame_cbd(cgv::render::context& ctx, bool is_right)
	{
		if (wall_kit_ptr == 0)
			return;

		if (wall_state == WS_HMD)
			return;

		GLint draw_buffer, draw_fbo;
		glGetIntegerv(GL_DRAW_BUFFER, &draw_buffer);
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_fbo);
			
		int width = ctx.get_width(), height = ctx.get_height();
		int x_off = 0, y_off = 0;
		if (stereo_window_mode == SWM_DOUBLE) {
			width /= 2;
			x_off = width;
		}
		double aspect = double(width) / height;

		int w = width, h = height;
		if (stereo_window_mode == SWM_SINGLE) {
			switch (stereo_shader_mode) {
			case SSM_SIDE_BY_SIDE: w /= 2; x_off = w; break;
			case SSM_TOP_BOTTOM: h /= 2; y_off = h; break;
			}
		}
		double a = double(w) / h;

		if (blit_fbo == -1) {
			glGenFramebuffers(1, &blit_fbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, blit_fbo);
			glGenTextures(2, blit_tex);
			for (int i = 0; i < 2; ++i) {
				glBindTexture(GL_TEXTURE_2D, blit_tex[i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
				glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, blit_tex[i], 0);
			}
		}
		else
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, blit_fbo);

		glBindTexture(GL_TEXTURE_2D, blit_tex[0]);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, w, h, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			
		glBindTexture(GL_TEXTURE_2D, blit_tex[1]);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glBlitFramebuffer(x_off, y_off, x_off+w, y_off+h, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, 0);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_fbo);
		glDrawBuffer(draw_buffer);
	}
}

#include <cgv/base/register.h>

/// register a newly created vr_wall instance
cgv::base::object_registration<vr::vr_wall> wall_reg("");

