#include <cgv/base/base.h>
#include "vr_wall.h"
#include <vr/vr_driver.h>
#include <cgv/media/image/image_reader.h>
#include <cgv/render/attribute_array_binding.h>
#include <cg_vr/vr_server.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/gui/key_event.h>
#include <libs/cgv_reflect_types/math/quaternion.h>
#include <cgv/gui/application.h>
#include <cgv/math/ftransform.h>
#include <random>

///
namespace vr {
	/// helper function to create the window for the wall display
	void vr_wall::create_wall_window()
	{
		window = cgv::gui::application::create_window(2 * window_width, window_height, "wall window", "viewer");
		window->set_name("wall_window");
		window->set("menu", false);
		window->set("gui", false);
		// optionally put it on specific monitor:
		//		window->set("state", "fullscreen(1)");
		//		window->set("state", "regular");
		window->set("w", 2 * window_width);
		window->set("h", window_height);
		window->set("x", window_x);
		window->set("y", window_y);
		window->show();
		window->register_object(this, "");
	}

	void vr_wall::generate_screen_calib_points()
	{
		float aspect = (float)window_width / window_height;
		points.push_back(vec3(0, 0, 0));
		colors[0].push_back(rgb(0, 1, 0));
		colors[1].push_back(rgb(0, 1, 0));
		points.push_back(vec3(aspect, 0, 0));
		colors[0].push_back(rgb(0, 1, 0));
		colors[1].push_back(rgb(0, 1, 0));
		points.push_back(vec3(0, 1, 0));
		colors[0].push_back(rgb(0, 1, 0));
		colors[1].push_back(rgb(0, 1, 0));
	}
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
	///
	void vr_wall::generate_eye_calib_points()
	{
		points.clear();
		colors[0].clear();
		colors[1].clear();
		points.push_back(vec3(-0.2f, 0, 0));
		colors[0].push_back(rgb(1, 0.5f, 0.5f));
		colors[1].push_back(rgb(1, 0.5f, 0.5f));
		points.push_back(vec3(0.2f, 0, 0));
		colors[0].push_back(rgb(0.5f, 0.5f, 1));
		colors[1].push_back(rgb(0.5f, 0.5f, 1));
	}
	/// add screen center sphere, x & y arrows and box for extruded screen rectangle
	void vr_wall::add_screen(const vec3& center, const vec3& x, const vec3& y, const rgb& clr, float lum)
	{
		sphere_positions.push_back(center);
		sphere_radii.push_back(0.025f);
		sphere_colors.push_back(rgb(lum, lum, lum));
		arrow_positions.push_back(center);
		arrow_directions.push_back(x);
		arrow_colors.push_back(rgb(1, lum, lum));
		arrow_positions.push_back(center);
		arrow_directions.push_back(y);
		arrow_colors.push_back(rgb(lum, 1, lum));
		boxes.push_back(box3(vec3(-x.length(), -y.length(), -0.01f), vec3(x.length(), y.length(), 0)));
		box_colors.push_back(clr);
		box_translations.push_back(center);
		mat3 R;
		vec3 x_dir = x; x_dir.normalize();
		vec3 y_dir = y; y_dir.normalize();
		vec3 z_dir = cross(x_dir, y_dir);
		R.set_col(0, x_dir);
		R.set_col(1, y_dir);
		R.set_col(2, z_dir);
		box_rotations.push_back(quat(R));
	}
	/// recompute the geometry based on current available  screens
	void vr_wall::rebuild_screens()
	{
		sphere_colors.clear();
		sphere_positions.clear();
		sphere_radii.clear();
		arrow_colors.clear();
		arrow_directions.clear();
		arrow_positions.clear();
		box_colors.clear();
		boxes.clear();
		box_rotations.clear();
		box_translations.clear();
		add_screen(test_screen_center, test_screen_x, test_screen_y, rgb(0.5f, 0.3f, 0.1f), 0.3f);
		add_screen(screen_center, screen_x, screen_y, rgb(0.5f, 0.7f, 0.3f), 0.5f);
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
		rebuild_screens();
		update_member(&wall_kit_ptr->pixel_size[0]);
		update_member(&wall_kit_ptr->pixel_size[1]);
		update_member(&screen_orientation[0]);
		update_member(&screen_orientation[1]);
		update_member(&screen_orientation[2]);
		update_member(&screen_orientation[3]);
		if (wall_state == WS_EYES_CALIB) {
			for (int ci = 0; ci < 2; ++ci) {
				points[ci] = wall_kit_ptr->transform_world_to_screen(c_P[ci] * vec4(aim_center, 1.0f));
				points[ci][2] = 0;
			}
		}
	}
	/// construct vr wall kit by attaching to another vr kit
	vr_wall::vr_wall() : cgv::base::node("wall")
	{		
		blit_fbo = -1;

		wall_kit_ptr = 0;
		test_screen_center = vec3(0, 1, 2);
		test_screen_x = vec3(-1.5f, 0, 0);
		test_screen_y = vec3(0, 1.0f, 0);
		ars.radius_relative_to_length = 0;
		ars.radius_lower_bound = 0.02f;
		peek_point = vec3(0, -0.0755634f, -0.0385344f);
		aim_direction = vec3(0, -0.531592f, 0.847f);
		aim_circle_radius = 0.0185f;
		aim_center = vec3(0, -0.0342337f, -0.0142778f);

		main_context = 0;
		vr_wall_kit_index = -1;
		vr_wall_hmd_index = -1;
		screen_orientation = quat(1, 0, 0, 0);		
		screen_center = vec3(0, 0.8f, 1);
		screen_x = vec3(-0.75f, 0, 0);
		screen_y = vec3(0, 0.5f, 0);
		window_width = 1920;
		window_height = 1080;
		window_x = 0;
		window_y = 0;
		prs.halo_color = rgba(0, 0, 0, 0.9f);
		prs.halo_width_in_pixel = -2.0f;
		prs.point_size = 15.0f;
		cgv::signal::connect(cgv::gui::ref_vr_server().on_device_change, this, &vr_wall::on_device_change);
		kit_enum_definition = "enums='none=-1";

		wall_state = WS_SCREEN_CALIB;
		rebuild_screens();
		on_set(&wall_state);
	}
	///
	vr_wall::~vr_wall()
	{
		cgv::base::unregister_object(window);
		cgv::gui::application::remove_window(window);
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
	///
	std::string vr_wall::get_type_name() const 
	{
		return "vr_wall"; 
	}
	///
	bool vr_wall::self_reflect(cgv::reflect::reflection_handler& srh)
	{
		return
			srh.reflect_member("vr_wall_kit_index", vr_wall_kit_index) &&
			srh.reflect_member("vr_wall_hmd_index", vr_wall_hmd_index) &&
			srh.reflect_member("screen_orientation", screen_orientation) &&
			srh.reflect_member("screen_center", screen_center) &&
			srh.reflect_member("screen_x", screen_x) &&
			srh.reflect_member("screen_y", screen_y) &&
			srh.reflect_member("wall_state", (int&)wall_state) &&
			srh.reflect_member("peek_point_x", peek_point[0]) &&
			srh.reflect_member("peek_point_y", peek_point[1]) &&
			srh.reflect_member("peek_point_z", peek_point[2]) &&
			srh.reflect_member("creation_width", window_width) &&
			srh.reflect_member("creation_height", window_height) &&
			srh.reflect_member("creation_x", window_x) &&
			srh.reflect_member("creation_y", window_y);
	}
	///
	void vr_wall::on_set(void* member_ptr)
	{
		if (member_ptr == &calib_index) {
			if (wall_state == WS_SCREEN_CALIB && calib_index == points.size()) {
				wall_state = WS_EYES_CALIB;
				on_set(&wall_state);
			}
		}
		else if (member_ptr == &wall_state) {
			points.clear();
			colors[0].clear();
			colors[1].clear();
			switch (wall_state) {
			case WS_SCREEN_CALIB:
				generate_screen_calib_points();
				calib_index = 0;
				update_member(&calib_index);
				break;
			case WS_EYES_CALIB:
				generate_eye_calib_points();
				//if (!generate_points_from_image("res://cgv.png", 0.5f))
				//	generate_points(20);
				calib_index = 0;
				break;
			case WS_HMD:
				break;
			}
		}
		else if (member_ptr == &vr_wall_kit_index) {
			if (vr_wall_kit_index >= 0) {
				std::vector<void*> kits = vr::scan_vr_kits();
				if (vr_wall_kit_index < (int)kits.size()) {
					if (wall_kit_ptr == 0) {
						wall_kit_ptr = new vr::vr_wall_kit(vr_wall_kit_index, window_width, window_height, "vr_wall_kit");
						on_update_screen_calibration();
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
		post_redraw();
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
		add_gui("peek_point", peek_point, "", "options='min=-0.1;max=0.1;step=0.0001;ticks=true'");
		if (begin_tree_node("test screen", test_screen_center, false, "level=2")) {
			align("\a");
			add_gui("test_screen_center", test_screen_center, "", "long_label=true;options='min-3;max=3;ticks=true'");
			add_gui("test_screen_x", test_screen_x, "", "long_label=true;options='min-3;max=3;ticks=true'");
			add_gui("test_screen_y", test_screen_y, "", "long_label=true;options='min-3;max=3;ticks=true'");
			if (begin_tree_node("box render style", brs, false, "level=3")) {
				align("\a");
				add_gui("box_render_style", brs);
				align("\b");
				end_tree_node(brs);
			}
			if (begin_tree_node("sphere render style", srs, false, "level=3")) {
				align("\a");
				add_gui("sphere_render_style", srs);
				align("\b");
				end_tree_node(brs);
			}
			if (begin_tree_node("arrow render style", ars, false, "level=3")) {
				align("\a");
				add_gui("arrow_render_style", ars);
				align("\b");
				end_tree_node(ars);
			}

			align("\b");
			end_tree_node(test_screen_center);
		}
		if (vr_wall_kit_index != -1 && wall_kit_ptr != 0) {
			add_decorator("screen", "heading", "level=2");
			add_view("width", wall_kit_ptr->width, "", "w=40", " ");
			add_view("height", wall_kit_ptr->height, "", "w=40", " ");
			add_view("multi", wall_kit_ptr->nr_multi_samples, "", "w=20");
			add_member_control(this, "pixel_size_x", wall_kit_ptr->pixel_size[0], "value_slider", "min=0.0001;max=0.01;ticks=true;log=true;step=0.00001");
			add_member_control(this, "pixel_size_y", wall_kit_ptr->pixel_size[1], "value_slider", "min=0.0001;max=0.01;ticks=true;log=true;step=0.00001");
			add_gui("screen_center", screen_center, "", "long_label=true;options='min=-3;max=3;ticks=true'");
			add_gui("screen_x", screen_x, "", "long_label=true;options='min=-3;max=3;ticks=true'");
			add_gui("screen_y", screen_y, "", "long_label=true;options='min=-3;max=3;ticks=true'");
			add_gui("screen_orientation", (vec4&)screen_orientation, "direction", "long_label=true;options='min=-1;max=1;ticks=true'");
			add_decorator("head", "heading", "level=2");
			add_member_control(this, "eye_separation", wall_kit_ptr->eye_separation, "value_slider", "min=0.01;max=0.12;ticks=true;step=0.001");
			add_gui("eye_center_tracker", wall_kit_ptr->eye_center_tracker, "", "options='min=-0.2;max=0.2;step=0.001;ticks=true'");
			add_gui("eye_separation_dir_tracker", wall_kit_ptr->eye_separation_dir_tracker, "direction", "options='min=-1;max=1;ticks=true'");
		}

	}
	///
	bool vr_wall::handle(cgv::gui::event& e)
	{
		if (e.get_kind() == cgv::gui::EID_POSE) {
			if ((e.get_flags() & cgv::gui::EF_VR) != 0) {
				auto& vrpe = dynamic_cast<cgv::gui::vr_pose_event&>(e);
				int ci = vrpe.get_trackable_index();
				if (ci >= 0 && ci < 2) {
					c_P[ci] = vrpe.get_pose_matrix();
					if (wall_state == WS_EYES_CALIB && wall_kit_ptr) {
						points[ci] = wall_kit_ptr->transform_world_to_screen(c_P[ci] * vec4(aim_center, 1.0f));
						points[ci][2] = 0;
						post_redraw();
					}
				}
			}
		}
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
					vec3 p = reinterpret_cast<const mat34&>(vrke.get_state().controller[vrke.get_controller_index()].pose[0])*vec4(peek_point, 1.0f);
					switch (calib_index) {
					case 0:
						screen_center = p;
						on_update_screen_calibration();
						break;
					case 1:
						screen_x = p - screen_center;
						on_update_screen_calibration();
						break;
					case 2:
						screen_y = p - screen_center;
						on_update_screen_calibration();
						break;
					}
					++calib_index;
					on_set(&calib_index);
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
	void vr_wall::init_frame(cgv::render::context& ctx)
	{
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
		if (main_context == 0) {
			main_context = &ctx;
			cgv::render::ref_box_renderer(ctx, 1);
			cgv::render::ref_sphere_renderer(ctx, 1);
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
			cgv::render::ref_sphere_renderer(ctx, -1);
			wall_kit_ptr->wall_context = true;
			wall_kit_ptr->destruct_fbos();
			wall_kit_ptr->wall_context = false;
			cgv::render::ref_box_renderer(ctx, -1);
			cgv::render::ref_sphere_renderer(ctx, -1);
			cgv::render::ref_arrow_renderer(ctx, -1);
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
			if (wall_kit_ptr) {
				std::vector<vec3> P;
				std::vector<vec2> T;
				vec3 z = cross(screen_x, screen_y);
				z.normalize();
				z *= 0.001f;
				P.push_back(screen_center + z - screen_x - screen_y); T.push_back(vec2(0.0f, 0.0f));
				P.push_back(screen_center + z + screen_x - screen_y); T.push_back(vec2(1.0f, 0.0f));
				P.push_back(screen_center + z - screen_x + screen_y); T.push_back(vec2(0.0f, 1.0f));
				P.push_back(screen_center + z + screen_x + screen_y); T.push_back(vec2(1.0f, 1.0f));
				auto& prog = ctx.ref_default_shader_program(true);
				prog.enable(ctx);
				prog.set_uniform(ctx, "texture", 0);
				ctx.set_color(rgba(1, 1, 1, 1));
				glActiveTexture(GL_TEXTURE0);
				wall_kit_ptr->bind_texture(0);
				cgv::render::attribute_array_binding::set_global_attribute_array(ctx, prog.get_position_index(), P);
				cgv::render::attribute_array_binding::enable_global_array(ctx, prog.get_position_index());
				cgv::render::attribute_array_binding::set_global_attribute_array(ctx, prog.get_texcoord_index(), T);
				cgv::render::attribute_array_binding::enable_global_array(ctx, prog.get_texcoord_index());
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				cgv::render::attribute_array_binding::disable_global_array(ctx, prog.get_position_index());
				cgv::render::attribute_array_binding::disable_global_array(ctx, prog.get_texcoord_index());
				prog.disable(ctx);
			}
			if (true) { //!sphere_positions.empty()) {
				int ci;
				for (ci = 0; ci < 2; ++ci) {
					sphere_positions.push_back(c_P[ci] * vec4(peek_point, 1.0f));
					sphere_colors.push_back(rgb(0, 1, 0));
					sphere_radii.push_back(0.003f);
					sphere_positions.push_back(c_P[ci] * vec4(aim_center, 1.0f));
					sphere_colors.push_back(rgb(0, 1, 1));
					sphere_radii.push_back(0.003f);
					sphere_positions.push_back(c_P[ci] * vec4(aim_center+0.1f*aim_direction, 1.0f));
					sphere_colors.push_back(rgb(0, 0, 1));
					sphere_radii.push_back(0.003f);
				}
				auto& sr = cgv::render::ref_sphere_renderer(ctx);
				sr.set_render_style(srs);
				sr.set_y_view_angle(45.0f);
				sr.set_position_array(ctx, sphere_positions);
				sr.set_radius_array(ctx, sphere_radii);
				sr.set_color_array(ctx, sphere_colors);
				if (sr.validate_and_enable(ctx)) {
					glDrawArrays(GL_POINTS, 0, (GLsizei)sphere_positions.size());
					sr.disable(ctx);
				}
				for (ci = 0; ci < 6; ++ci) {
					sphere_positions.pop_back();
					sphere_colors.pop_back();
					sphere_radii.pop_back();
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

		int width = ctx.get_width()/2;
		int height = ctx.get_height();
		switch (wall_state) {
		case WS_SCREEN_CALIB:
		case WS_EYES_CALIB:
			for (int eye = 0; eye < 2; ++eye) {
				ctx.set_viewport(ivec4(eye*width, 0, width, height));
				ctx.set_projection_matrix(cgv::math::perspective4<double>(90, (double)width/height, 0.1, 10.0));
				ctx.set_modelview_matrix(cgv::math::look_at4<double>(dvec3(0, 0, 1), dvec3(0, 0, 0), dvec3(0, 1, 0)));

				pr.set_y_view_angle(90);
				pr.set_render_style(prs);
				pr.set_position_array(ctx, points);
				pr.set_color_array(ctx, colors[eye]);
				if (pr.validate_and_enable(ctx)) {
					if (wall_state == WS_SCREEN_CALIB)
						glDrawArrays(GL_POINTS, calib_index, 1);
					else
						glDrawArrays(GL_POINTS, 0, points.size());
					pr.disable(ctx);
				}
			}
			break;
		case WS_HMD:
			wall_kit_ptr->wall_context = true;
			wall_kit_ptr->blit_fbo(0, 0, 0, width, height);
			wall_kit_ptr->blit_fbo(1, width, 0, width, height);
			wall_kit_ptr->wall_context = false;
			break;
		}
		return;
	}
	///
	void vr_wall::finish_frame(cgv::render::context& ctx)
	{
		if (&ctx != main_context && (wall_state != WS_HMD)) {
			GLint draw_buffer, draw_fbo;
			glGetIntegerv(GL_DRAW_BUFFER, &draw_buffer);
			glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_fbo);


			if (blit_fbo == -1) {
				glGenFramebuffers(1, &blit_fbo);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, blit_fbo);
				wall_kit_ptr->bind_texture(0);
				GLint tex_id;
				glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex_id);
				glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0);
				wall_kit_ptr->bind_texture(1);
				glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex_id);
				glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex_id, 0);
			}
			else
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, blit_fbo);

			wall_kit_ptr->bind_texture(0);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			
			glBlitFramebuffer(0, 0, ctx.get_width() / 2, ctx.get_height(), 0, 0, ctx.get_width() / 2, ctx.get_height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
			wall_kit_ptr->bind_texture(1);
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
			glBlitFramebuffer(ctx.get_width() / 2, 0, ctx.get_width() / 2, ctx.get_height(), 0, 0, ctx.get_width() / 2, ctx.get_height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

			glBindTexture(GL_TEXTURE_2D, 0);

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_fbo);
			glDrawBuffer(draw_buffer);
		}
	}
}

#include <cgv/base/register.h>

/// register a newly created vr_wall instance
cgv::base::object_registration<vr::vr_wall> wall_reg("");

