#include <cgv/base/base.h>
#include "vr_wall.h"
#include <vr/vr_driver.h>
#include <cgv/media/image/image_reader.h>
#include <cg_vr/vr_server.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/gui/key_event.h>
#include <libs/cgv_reflect_types/math/quaternion.h>
#include <cgv/gui/application.h>
#include <cgv/math/ftransform.h>
#include <random>
///
namespace vr {

	void vr_wall::generate_points(int n)
	{
		std::default_random_engine E;
		std::uniform_real_distribution<float> D(0.0f, 1.0f);
		for (int i = 0; i < n; ++i) {
			points.push_back(vec3(2.0f*vec2(D(E), D(E))-1.0f, 0.0f));
			float f = 0.5f*D(E) + 0.5f;
			colors[0].push_back(rgb(f, 0.5f, 0.5f));
			colors[1].push_back(rgb(0.5f, 0.5f, f));
		}
	}
	bool vr_wall::generate_points_from_image(const std::string& file_name, float angle)
	{
		cgv::data::data_format df;
		cgv::data::data_view dv;
		cgv::media::image::image_reader ir(df);
		if (!ir.open(file_name))
			return false;
		if (!ir.read_image(dv)) {
			ir.close();
			return false;
		}
		std::default_random_engine E;
		std::uniform_real_distribution<float> D(0.0f, 1.0f);
		int w = (int)df.get_width();
		int h = (int)df.get_height();
		float scale = 1.0f / h;
		uint8_t* data_ptr = dv.get_ptr<uint8_t>();
		int pixel_size = df.get_entry_size();
		float c = cos(angle);
		float s = sin(angle);
		mat2 R;
		R.set_col(0, vec2(c, s));
		R.set_col(1, vec2(-s, c));

		for (int i = 0; i < w; ++i) {
			for (int j = 0; j < h; ++j) {
				uint8_t* pixel_ptr = data_ptr + ((j*w + i)*pixel_size);
				if (pixel_ptr[0] > 80) {
					vec2 p(scale*(i - w / 2), scale*(h / 2 - j));
					points.push_back(vec3(2.2f*R*p, 0.0f));
					float f = 0.5f*D(E) + 0.5f;
					colors[0].push_back(rgb(f, 0.5f, 0.5f));
					colors[1].push_back(rgb(0.5f, 0.5f, f));
				}
			}
		}
		return true;
	}
	void vr_wall::generate_screen_calib_points()
	{
		points.push_back(vec3(0, 0, 0));
		colors[0].push_back(rgb(0, 1, 0));
		colors[1].push_back(rgb(0, 1, 0));
		points.push_back(vec3(1, 0, 0));
		colors[0].push_back(rgb(0, 1, 0));
		colors[1].push_back(rgb(0, 1, 0));
		points.push_back(vec3(0, 1, 0));
		colors[0].push_back(rgb(0, 1, 0));
		colors[1].push_back(rgb(0, 1, 0));
	}
	/// construct vr wall kit by attaching to another vr kit
	vr_wall::vr_wall() : cgv::base::node("wall")
	{		
		test_screen_center = vec3(0, 1, 2);
		test_screen_x = vec3(-1.5f, 0, 0);
		test_screen_y = vec3(0, 1.0f, 0);
		add_arrows(test_screen_center, test_screen_x, test_screen_y, 0.3f);
		add_screen_box(test_screen_center, test_screen_x, test_screen_y, rgb(0.5f,0.3f,0.1f));
		ars.radius_relative_to_length = 0;
		ars.radius_lower_bound = 0.02f;

		main_context = 0;
		vr_wall_kit_index = -1;
		vr_wall_hmd_index = -1;
		wall_kit_ptr = 0;
		screen_orientation = quat(1, 0, 0, 0);		
		window_width = 320;
		window_height = 240;
		window_x = 0;
		window_y = 0;
		prs.halo_color = rgba(0, 0, 0, 0.9f);
		prs.halo_width_in_pixel = -2.0f;
		prs.point_size = 15.0f;
		cgv::signal::connect(cgv::gui::ref_vr_server().on_device_change, this, &vr_wall::on_device_change);
		kit_enum_definition = "enums='none=-1";

		wall_state = WS_SCREEN_CALIB;
		on_set(&wall_state);
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
		window = cgv::gui::application::create_window(2 * window_width, window_height, "wall window", "viewer");
		window->set_name("wall_window");
//		cgv::base::register_object(window);
		window->set("menu", false);
		window->set("gui", false);
		window->set("w", 2 * window_width);
		window->set("h", window_height);
		window->set("x", window_x);
		window->set("y", window_y);
		window->show();
		//cgv::base::node_ptr parent = get_parent();
		window->register_object(this, "");
		//cgv::base::node::set_parent(parent);
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

		if (begin_tree_node("window creation parameters", window_width, true, "level=2")) {
			align("\a");
			add_member_control(this, "width", window_width, "value_slider", "min=320;max=3920;ticks=true;log=true");
			add_member_control(this, "height", window_height, "value_slider", "min=240;max=2160;ticks=true;log=true");
			add_member_control(this, "x", window_x, "value_slider", "min=0;max=3920;ticks=true;log=true");
			add_member_control(this, "y", window_y, "value_slider", "min=0;max=2160;ticks=true;log=true");
			align("\b");
			end_tree_node(window_width);
		}
		if (begin_tree_node("points", prs, false, "level=2")) {
			align("\a");
			add_gui("point_style", prs);
			align("\b");
			end_tree_node(prs);
		}
		if (begin_tree_node("test screen", test_screen_center, false, "level=2")) {
			align("\a");
			add_gui("test_screen_center", test_screen_center, "", "options='min-3;max=3;ticks=true'");
			add_gui("test_screen_x", test_screen_x, "", "options='min-3;max=3;ticks=true'");
			add_gui("test_screen_y", test_screen_y, "", "options='min-3;max=3;ticks=true'");
			add_decorator("box rendering", "heading", "level=3");
			align("\a");
			add_gui("box_render_style", brs);
			align("\b");
			add_decorator("arrow rendering", "heading", "level=3");
			align("\a");
			add_gui("arrow_render_style", ars);
			align("\b");
			align("\b");
			end_tree_node(test_screen_center);
		}
		if (vr_wall_kit_index != -1 && wall_kit_ptr != 0) {
			add_decorator("screen", "heading", "level=2");
			add_view("width", wall_kit_ptr->width, "", "w=60", " ");
			add_view("height", wall_kit_ptr->height, "" "w=60", " ");
			add_view("multi", wall_kit_ptr->nr_multi_samples, "" "w=30");
			add_member_control(this, "pixel_size", wall_kit_ptr->pixel_size, "value_slider", "min=0.0001;max=0.01;ticks=true;log=true;step=0.00001");
			add_gui("screen_center", wall_kit_ptr->screen_center_world, "", "options='min=-3;max=3;ticks=true'");
			add_gui("screen_orientation", screen_orientation, "direction", "options='min=-1;max=1;ticks=true'");
			add_decorator("head", "heading", "level=2");
			add_member_control(this, "eye_separation", wall_kit_ptr->eye_separation, "value_slider", "min=0.01;max=0.12;ticks=true;step=0.001");
			add_gui("eye_center_tracker", wall_kit_ptr->eye_center_tracker, "", "options='min=-0.2;max=0.2;step=0.001;ticks=true'");
			add_gui("eye_separation_dir_tracker", wall_kit_ptr->eye_separation_dir_tracker, "direction", "options='min=-1;max=1;ticks=true'");
		}

	}
	
	///
	bool vr_wall::handle(cgv::gui::event& e)
	{
		if (e.get_kind() != cgv::gui::EID_KEY)
			return false;
		auto& ke = dynamic_cast<cgv::gui::key_event&>(e);
		if (ke.get_action() == cgv::gui::KA_RELEASE)
			return false;
		if (ke.get_flags()&cgv::gui::EF_VR) {
			auto& vrke = dynamic_cast<cgv::gui::vr_key_event&>(ke);
			switch (vrke.get_key()) {
			case vr::VR_LEFT_BUTTON0:
			case vr::VR_RIGHT_BUTTON0:
				switch (wall_state) {
				case WS_SCREEN_CALIB :
				{
					vec3 p = reinterpret_cast<const mat34&>(vrke.get_state().controller[vrke.get_controller_index()].pose[0])*vec4(0, 0, -0.3f, 1.0f);
					switch (calib_point_index) {
					case 0:
						wall_kit_ptr->screen_center_world = p;
						break;
					case 1:
						wall_kit_ptr->screen_x_world = p - wall_kit_ptr->screen_center_world;
						break;
					case 2:
						wall_kit_ptr->screen_y_world = p - wall_kit_ptr->screen_center_world;
						break;
					}
					++calib_point_index;
					on_set(&calib_point_index);
					return true;
				}
				}
				break;
			}
		}
		else {
			switch (ke.get_key()) {
			case 'S':
				wall_state = WS_SCREEN_CALIB;
				on_set(&wall_state);
				return true;
			case 'E':
				wall_state = WS_EYES_CALIB;
				on_set(&wall_state);
				return true;
			case 'H':
				wall_state = WS_HMD;
				on_set(&wall_state);
				return true;
			}
		}
		return false;
	}
	///
	void vr_wall::stream_help(std::ostream& os)
	{
		os << "vr_wall: no help yet" << std::endl;
	}

	///
	void vr_wall::on_set(void* member_ptr)
	{
		if (member_ptr == &calib_point_index) {
			if (wall_state == WS_SCREEN_CALIB && calib_point_index == points.size()) {
				wall_state = WS_EYES_CALIB;
				on_set(&wall_state);
			}
			return;
		}
		if (member_ptr == &wall_state) {
			points.clear();
			colors[0].clear();
			colors[1].clear();
			switch (wall_state) {
			case WS_SCREEN_CALIB:
				generate_screen_calib_points();
				calib_point_index = 0;
				update_member(&calib_point_index);
				break;
			case WS_EYES_CALIB:
				if (!generate_points_from_image("res://cgv.png", 0.5f))
					generate_points(100);
				eye_calibrated[0] = eye_calibrated[1] = false;
				break;
			case WS_HMD:
				break;
			}
			return;
		}
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
	void vr_wall::init_frame(cgv::render::context& ctx)
	{
		/*
		std::cout << "init_frame(";
		if (&ctx == main_context)
			std::cout << "main";
		else
			std::cout << "wall";
		std::cout << ") : " << wglGetCurrentContext() << std::endl;
		*/

		if (&ctx == main_context)
			return;

		wall_kit_ptr->wall_context = true;
		if (!wall_kit_ptr->fbos_initialized())
			if (wall_kit_ptr->init_fbos())
				std::cout << "initialized fbos of wall kit in context " << (void*)&ctx << std::endl;
		wall_kit_ptr->wall_context = false;
	}
	///
	bool vr_wall::init(cgv::render::context& ctx)
	{
		/*
		std::cout << "init(";
		if (&ctx == main_context)
			std::cout << "main";
		else
			std::cout << "wall";
		std::cout << ") : " << wglGetCurrentContext() << std::endl;
		*/
		if (main_context == 0) {
			main_context = &ctx;
			cgv::render::ref_box_renderer(ctx, 1);
			cgv::render::ref_arrow_renderer(ctx, 1);
		}
		else {
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
		}
		return true;
	}
	///
	void vr_wall::clear(cgv::render::context& ctx)
	{
		if (&ctx == main_context) {
			cgv::render::ref_box_renderer(ctx, -1);
			cgv::render::ref_arrow_renderer(ctx, -1);
			wall_kit_ptr->wall_context = true;
			wall_kit_ptr->destruct_fbos();
			wall_kit_ptr->wall_context = false;
		}
		else {
			if (wall_kit_ptr) {
				wall_kit_ptr->destruct_fbos();
				delete wall_kit_ptr;
				wall_kit_ptr = 0;
			}
			pr.clear(ctx);
		}
	}
	///
	void vr_wall::draw(cgv::render::context& ctx)
	{
		/*
		std::cout << "draw(";
		if (&ctx == main_context)
			std::cout << "main";
		else
			std::cout << "wall";
		std::cout << ") : " << wglGetCurrentContext() << std::endl;
		*/
		if (&ctx == main_context) {
			if (!boxes.empty()) {
				auto& br = cgv::render::ref_box_renderer(ctx);
				br.set_render_style(brs);
				br.set_box_array(ctx, boxes);
				br.set_color_array(ctx, box_colors);
				br.set_translation_array(ctx, box_translations);
				br.set_rotation_array(ctx, box_rotations);
				if (br.validate_and_enable(ctx)) {
					glDrawArrays(GL_POINTS, 0, (GLsizei)boxes.size());
					br.disable(ctx);
				}
			}
			if (!arrow_positions.empty()) {
				auto& ar = cgv::render::ref_arrow_renderer(ctx);
				ar.set_render_style(ars);
				ar.set_position_array(ctx, arrow_positions);
				ar.set_direction_array(ctx, arrow_directions);
				ar.set_color_array(ctx, arrow_colors);
				if (ar.validate_and_enable(ctx)) {
					glDrawArraysInstanced(GL_POINTS, 0, (GLsizei)arrow_positions.size(), ars.nr_subdivisions);
					ar.disable(ctx);
				}
			}
			return;
		}
		if (window.empty() || wall_kit_ptr == 0)
			return;

		int width = ctx.get_width();
		int height = ctx.get_height();
		switch (wall_state) {
		case WS_SCREEN_CALIB:
		case WS_EYES_CALIB:
			for (int eye = 0; eye < 2; ++eye) {
				ctx.set_viewport(ivec4(eye*width / 2, 0, width / 2, height));
				ctx.set_projection_matrix(cgv::math::perspective4<double>(90, (double)width / (2 * height), 0.1, 10.0));
				ctx.set_modelview_matrix(cgv::math::look_at4<double>(dvec3(0, 0, 1), dvec3(0, 0, 0), dvec3(0, 1, 0)));

				pr.set_y_view_angle(90);
				pr.set_render_style(prs);
				pr.set_position_array(ctx, points);
				pr.set_color_array(ctx, colors[eye]);
				if (pr.validate_and_enable(ctx)) {
					if (wall_state == WS_SCREEN_CALIB)
						glDrawArrays(GL_POINTS, calib_point_index, 1);
					else
						glDrawArrays(GL_POINTS, 0, points.size());
					pr.disable(ctx);
				}
			}
			break;
		case WS_HMD:
			wall_kit_ptr->wall_context = true;
			wall_kit_ptr->blit_fbo(0, 0, 0, width / 2, height);
			wall_kit_ptr->blit_fbo(1, width / 2, 0, width / 2, height);
			wall_kit_ptr->wall_context = false;
			break;
		}
		return;

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
cgv::base::object_registration<vr::vr_wall> wall_reg("");// views = 'wall_window'");

