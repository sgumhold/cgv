#include "vr_stream_gui.h"
#include "debug_draw.h"
#include "mouse_win.h"
#include <random>
#include <cg_nui/ray_tool.h>
#include <cg_nui/point_tool.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/register.h>
#include <cgv/math/ftransform.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/options.h>
#include <cgv/gui/dialog.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/rectangle_renderer.h>
#include <cgv/media/mesh/simple_mesh.h>
#include <cg_vr/vr_events.h>

#include <random>

#include "intersection.h"

void vr_stream_gui::set_stream_screen()
{

	if (stream_screen) {
		screen_tex_manager->start_grabbing_async();
	}
	else {
		screen_tex_manager->stop_grabbing_async();
	}

	post_redraw();
}

void vr_stream_gui::set_draw_debug()
{
	vt_display->set_draw_plane(draw_debug);
	post_redraw();
}

void vr_stream_gui::reconfigure_virtual_display(vec3 pos)
{
	switch (vt_display_reconfigure_state) {
	case 0: {
		vt_display_reconfigure_first_pos = pos;
		break;
	}
	case 1: {
		vt_display_reconfigure_second_pos = pos;

		auto ul = vt_display_reconfigure_first_pos;
		auto dr = vt_display_reconfigure_second_pos;

		auto ur = vec3(dr.x(), ul.y(), dr.z());
		auto dl = vec3(ul.x(), dr.y(), ul.z());

		cgv::data::rectangle rect = { ul, ur, dl, dr };
		vr::room::plane plane(rect);

		vt_display->set_display(plane);
		// reset config values to zero, as not needed for dynamic positioning
		vt_display->set_position(vec3(0.0f));
		vt_display->set_orientation(quat(1.0f, 0.0f, 0.0f, 0.0f));

		break;
	}
	default:
		std::cerr << "invalid vt_display configuration state" << std::endl;
		break;
	}

	vt_display_reconfigure_state = (vt_display_reconfigure_state + 1) % 2;
	if (vt_display_reconfigure_state == 0) vt_display_reconfiguring = false;
	update_member(&vt_display_reconfiguring);
}

void vr_stream_gui::init_cameras(vr::vr_kit* kit_ptr)
{
	vr::vr_camera* camera_ptr = kit_ptr->get_camera();
	if (!camera_ptr)
		return;
	nr_cameras = camera_ptr->get_nr_cameras();
	frame_split = camera_ptr->get_frame_split();
	for (int i = 0; i < nr_cameras; ++i) {
		std::cout << "camera " << i << "(" << nr_cameras << "):" << std::endl;
		camera_ptr->put_camera_intrinsics(i, false, &focal_lengths[i](0), &camera_centers[i](0));
		camera_ptr->put_camera_intrinsics(i, true, &focal_lengths[2 + i](0), &camera_centers[2 + i](0));
		std::cout << "  fx=" << focal_lengths[i][0] << ", fy=" << focal_lengths[i][1] << ", center=[" << camera_centers[i] << "]" << std::endl;
		std::cout << "  fx=" << focal_lengths[2+i][0] << ", fy=" << focal_lengths[2+i][1] << ", center=[" << camera_centers[2+i] << "]" << std::endl;
		float camera_to_head[12];
		camera_ptr->put_camera_to_head_matrix(i, camera_to_head);
		kit_ptr->put_eye_to_head_matrix(i, camera_to_head);
		camera_to_head_matrix[i] = vr::get_mat4_from_pose(camera_to_head);
		std::cout << "  C2H=" << camera_to_head_matrix[i] << std::endl;
		camera_ptr->put_projection_matrix(i, false, 0.001f, 10.0f, &camera_projection_matrix[i](0, 0));
		camera_ptr->put_projection_matrix(i, true, 0.001f, 10.0f, &camera_projection_matrix[2+i](0, 0));
		std::cout << "  dP=" << camera_projection_matrix[i] << std::endl;
		std::cout << "  uP=" << camera_projection_matrix[2+i] << std::endl;
	}
	post_recreate_gui();
}

void vr_stream_gui::start_camera()
{
	if (!vr_view_ptr)
		return;
	vr::vr_kit* kit_ptr = vr_view_ptr->get_current_vr_kit();
	if (!kit_ptr)
		return;
	vr::vr_camera* camera_ptr = kit_ptr->get_camera();
	if (!camera_ptr)
		return;
	if (!camera_ptr->start())
		cgv::gui::message(camera_ptr->get_last_error());
}

void vr_stream_gui::stop_camera()
{
	if (!vr_view_ptr)
		return;
	vr::vr_kit* kit_ptr = vr_view_ptr->get_current_vr_kit();
	if (!kit_ptr)
		return;
	vr::vr_camera* camera_ptr = kit_ptr->get_camera();
	if (!camera_ptr)
		return;
	if (!camera_ptr->stop())
		cgv::gui::message(camera_ptr->get_last_error());
}

/// register on device change events
void vr_stream_gui::on_device_change(void* kit_handle, bool attach)
{
	if (attach) {
		if (last_kit_handle == 0) {
			vr::vr_kit* kit_ptr = vr::get_vr_kit(kit_handle);
			init_cameras(kit_ptr);
			if (kit_ptr) {
				last_kit_handle = kit_handle;
				post_recreate_gui();
			}
		}
	}
	else {
		if (kit_handle == last_kit_handle) {
			last_kit_handle = 0;
			post_recreate_gui();
		}
	}
}

/// construct boxes that can be moved around
void vr_stream_gui::construct_movable_boxes(cgv::nui::nui_primitive_node_ptr node, float tw, float td, float th, float tW, size_t nr)
{	
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0, 1);
	std::uniform_real_distribution<float> signed_distribution(-1, 1);
	for(size_t i = 0; i < nr; ++i) {
		float x = distribution(generator);
		float y = distribution(generator);
		vec3 extent(distribution(generator), distribution(generator), distribution(generator));
		extent += 0.1f;
		extent *= std::min(tw, td)*0.1f;

		vec3 center(-0.5f*tw + x * tw, th + tW, -0.5f*td + y * td);
		quat rot(signed_distribution(generator), signed_distribution(generator), signed_distribution(generator), signed_distribution(generator));
		rot.normalize();
		node->ref_boxes()->add_box(
			box3(center - 0.5f * extent, center + 0.5f * extent), rot,
			rgba(distribution(generator), distribution(generator), distribution(generator), 1.0f));
	}
}

vr_stream_gui::vr_stream_gui()
{

	screen_tex_manager = std::make_shared<util::screen_texture_manager>();
	intersection = {};
	ray_origin = vec3(0.0f);
	ray_dir = vec3(0.0f);

	frame_split = 0;
	extent_texcrd = vec2(0.5f, 0.5f);
	center_left = vec2(0.5f, 0.25f);
	center_right = vec2(0.5f, 0.25f);
	seethrough_gamma = 0.33f;
	frame_width = frame_height = 0;
	background_distance = 2;
	background_extent = 2;
	undistorted = true;
	shared_texture = true;
	max_rectangle = false;
	nr_cameras = 0;
	camera_tex_id = -1;
	camera_aspect = 1;
	use_matrix = true;
	show_seethrough = false;
	set_name("vr_stream_gui");
	vr_view_ptr = 0;
	last_kit_handle = 0;
	connect(cgv::gui::ref_vr_server().on_device_change, this, &vr_stream_gui::on_device_change);

	cgv::media::font::enumerate_font_names(font_names);
	font_enum_decl = "enums='";
	for (unsigned i = 0; i < font_names.size(); ++i) {
		if (i > 0)
			font_enum_decl += ";";
		std::string fn(font_names[i]);
		font_enum_decl += std::string(fn);
	}
	font_enum_decl += "'";

	construct_scene();
}

void vr_stream_gui::construct_scene()
{
	scene = new cgv::nui::nui_node("scene");
	lab = new cgv::nui::nui_primitive_node("lab");
	lab->set_interaction_capabilities(cgv::nui::IC_NONE);
	scene->append_child(lab);
	lab->create_box_container(true, false);
	lab->construct_lab(5, 7, 3, 0.2f, 1.6f, 0.8f, 0.7f, 0.03f);

	node = new cgv::nui::nui_primitive_node("content", cgv::nui::SM_UNIFORM);
	scene->append_child(node);
	node->create_box_container(true, true);
	node->create_rectangle_container(true, true, true);
	node->create_sphere_container(true, true);
	construct_movable_boxes(node, 1.6f, 0.8f, 0.7f, 0.03f, 20);
	construct_labels(node);
	node->ref_spheres()->add_sphere(vec3(-1, 0.5f, 0), rgba(1, 1, 0, 1), 0.1f);
	node->ref_spheres()->add_sphere(vec3(1, 0.5f, 0), rgba(1, 0, 1, 1), 0.15f);
	mesh_node = new cgv::nui::nui_mesh_node("mesh", cgv::nui::SM_UNIFORM);
	mesh_node->set_scale(vec3(0.001f));
	mesh_node->set_translation(vec3(-0.5f,0.9f,0.0f));
#ifdef _DEBUG
	mesh_node->set_file_name("D:/data/surface/meshes/obj/Max-Planck_lowres.obj");
#else
	mesh_node->set_file_name("D:/data/surface/meshes/obj/Max-Planck_highres.obj");
#endif
	scene->append_child(mesh_node);

	tools[0] = new cgv::nui::point_tool("left point", 0);
	tools[1] = new cgv::nui::ray_tool("right ray", 1);
	tools[0]->scene_node = scene;
	tools[1]->scene_node = scene;
	append_child(scene);
	append_child(tools[0]);
	append_child(tools[1]);
}

/// construct labels
void vr_stream_gui::construct_labels(cgv::nui::nui_primitive_node_ptr node)
{
	cgv::media::font::font_ptr f = cgv::media::font::find_font("Open Sans");
	lm.set_font_face(f->get_font_face(cgv::media::font::FFA_BOLD));
	lm.set_font_size(36);
	std::string content;
	std::default_random_engine r;
	std::uniform_real_distribution<float> d(0.0f, 1.0f);
	if (cgv::utils::file::read("D:/develop/projects/git/cgv/plugins/vr_stream_gui/labels.txt", content, true)) {
		std::vector<cgv::utils::line> lines;
		cgv::utils::split_to_lines(content, lines);
		for (auto& l : lines) {
			std::string line = to_string(l);			
			lm.add_label(line, rgba(cgv::media::color<float, cgv::media::HLS, cgv::media::OPACITY>(d(r), 0.5f, 1.0f)));
		}
	}
	lm.compute_label_sizes();
	lm.pack_labels();

	float scale = 0.002f;
	for (uint32_t i = 0; i < lm.get_nr_labels(); ++i) {
		const auto& l = lm.get_label(i);
		box2 b = reinterpret_cast<const box2&>(lm.get_texcoord_range(i));
		int j = i % 6;
		int k = i / 6;
		node->ref_rectangles()->add_rectangle(
			vec3(0.4f*(j-2.5f), 1 + 0.1f * k, 0.5f-0.2f*std::abs(j - 2.5f)),
			vec2(scale * l.get_width(), scale * l.get_height()), b,
			quat(vec3(0,1,0),0.2f*(j-2.5f)+3.14f), 
			l.background_color);
	}
	auto& srs = node->ref_rectangles()->ref_render_style();
	srs.illumination_mode = cgv::render::IM_OFF;
}
	
void vr_stream_gui::stream_help(std::ostream& os) {
	os << "vr_stream_gui: no shortcuts defined" << std::endl;
}
	
void vr_stream_gui::on_set(void* member_ptr)
{
	update_member(member_ptr);
	post_redraw();
}
	
bool vr_stream_gui::handle(cgv::gui::event& e)
{
	if (tools[0]->handle(e))
		return true;
	if (tools[1]->handle(e))
		return true;
	// check if vr event flag is not set and don't process events in this case
	if ((e.get_flags() & cgv::gui::EF_VR) == 0) {
		switch (e.get_kind()) {
		case cgv::gui::EID_KEY: {
			cgv::gui::key_event &ke = static_cast<cgv::gui::key_event &>(e);
			if (ke.get_action() == cgv::gui::KA_PRESS) {
				switch (ke.get_key()) {
				case 'V':
					prohibit_vr_input = !prohibit_vr_input;
					update_member(&prohibit_vr_input);
					if (prohibit_vr_input)
						vm_controller.try_fix_state();
					return true;
				case 'D':
					draw_debug = !draw_debug;
					update_member(&draw_debug);
					set_draw_debug();
					return true;
				case 'R':
					vt_display_reconfiguring = !vt_display_reconfiguring;
					update_member(&vt_display_reconfiguring);
					return true;
				case 'S':
					stream_screen = !stream_screen;
					set_stream_screen();
					update_member(&stream_screen);
					return true;
				}
			}
		}
		}
		return false;
	}

	// master-switch for vr events
	if (!prohibit_vr_input) {

		// master-switch for vt_display reconfiguration
		if (vt_display_reconfiguring) {
			if (e.get_kind() == cgv::gui::EID_THROTTLE) {
				cgv::gui::vr_throttle_event &vrte =
					static_cast<cgv::gui::vr_throttle_event &>(e);
				if (vrte.get_controller_index() == 1) {
					if (vrte.get_value() == 1.0f) {
						const vr::vr_kit_state *state_ptr =
							vr_view_ptr->get_current_vr_state();
						if (state_ptr) {
							auto pose = state_ptr->controller[1].pose;
							reconfigure_virtual_display({ pose[9], pose[10], pose[11] });
							return true;
						};
					}
				}
			}
		}

		if (vm_controller.handle(e))
			return true;

		// check event id
		switch (e.get_kind()) {
		case cgv::gui::EID_POSE: {

			// shoots ray into direction right controller is pointing to
			const vr::vr_kit_state *state_ptr = vr_view_ptr->get_current_vr_state();
			if (state_ptr) {

				cgv::gui::vr_pose_event &vrpe = static_cast<cgv::gui::vr_pose_event &>(e);

				// right controller
				{
					vec3 r_o, r_d;
					state_ptr->controller[switch_left_and_right_hand ? 0 : 1].put_ray(
						&r_o(0), &r_d(0));
					r_d.normalize();

					ray_origin = r_o;
					ray_dir = r_d;

					intersection = vt_display->intersect(ray_origin, ray_dir);

					/*std::cout << "shoot ray from (" << ray_origin.x() << ", "
								<< ray_origin.y() << ", " << ray_origin.z()
								<< ") to direction (" << ray_dir.x() << "," <<
					   ray_dir.y()
								<< "," << ray_dir.z() << ")" << std::endl;*/
					if (intersection.hit) {
						/*	std::cout << "hit at (" << vt_ints.pos_world.x() << ", "
										<< vt_ints.pos_world.y() << ", " <<
							vt_ints.pos_world.z()
										<< ") rel: (" << vt_ints.pos_rel.x() << "% "
										<< vt_ints.pos_rel.y() << "%) pix: ("
										<< vt_ints.pos_pixel.x() << "px " <<
							vt_ints.pos_pixel.y()
										<< "px)" << std::endl;*/

						util::mouse::set_position(intersection.pos_pixel.x(),
							intersection.pos_pixel.y());
						return true;
					}
				}
			}
		}
		break;
		}
	}
	return false;
}

bool vr_stream_gui::init(cgv::render::context& ctx)
{
	if (!cgv::utils::has_option("NO_OPENVR"))
		ctx.set_gamma(1.0f);

	if (!seethrough.build_program(ctx, "seethrough.glpr"))
		cgv::gui::message("could not build seethrough program");
	
	cgv::gui::connect_vr_server(true);

	auto view_ptr = find_view_as_node();
	if (view_ptr) {

		vt_display = std::make_shared<vr::room::virtual_display>();
		vt_display->set_display(
			{ {dvec3(-0.525 / 2.0, 0.295 / 2.0, 0.0), dvec3(0.525 / 2.0, 0.295 / 2.0, 0.0),
			  dvec3(-0.525 / 2.0, -0.295 / 2.0, 0.0),
			  dvec3(0.525 / 2.0, -0.295 / 2.0, 0.0)} });
		vt_display->set_resolution(1920u, 1080u);
		vt_display->init(ctx);

		auto blt = vt_display->get_blitter();
		if (auto blt_ptr = blt.lock()) {
			blt_ptr->set_texture(screen_tex_manager->get_texture());
		}

		view_ptr->set_eye_keep_view_angle(dvec3(0, 4, -4));
		// if the view points to a vr_view_interactor
		vr_view_ptr = dynamic_cast<vr_view_interactor*>(view_ptr);
		if (vr_view_ptr) {
			// configure vr event processing
			vr_view_ptr->set_event_type_flags(
				cgv::gui::VREventTypeFlags(
					cgv::gui::VRE_KEY +
					cgv::gui::VRE_THROTTLE +
					cgv::gui::VRE_STICK +
					cgv::gui::VRE_STICK_KEY +
					cgv::gui::VRE_POSE
				));
			vr_view_ptr->enable_vr_event_debugging(false);
			// configure vr rendering
			vr_view_ptr->draw_action_zone(false);
			vr_view_ptr->draw_vr_kits(true);
			vr_view_ptr->enable_blit_vr_views(true);
			vr_view_ptr->set_blit_vr_view_width(200);
		}
	}

	screen_tex_manager->recreate_texture(ctx);
	screen_tex_manager->set_fps_limit(60.0f);
	// always get first image
	screen_tex_manager->start_grabbing_async();
	screen_tex_manager->update(ctx);
	if (!stream_screen)
		screen_tex_manager->stop_grabbing_async();

	cgv::render::ref_box_renderer(ctx, 1);
	cgv::render::ref_sphere_renderer(ctx, 1);
	cgv::render::ref_cone_renderer(ctx, 1);
	cgv::render::ref_rectangle_renderer(ctx, 1);
	return true;
}

void vr_stream_gui::clear(cgv::render::context& ctx)
{
	lm.destruct(ctx);
	cgv::render::ref_box_renderer(ctx, -1);
	cgv::render::ref_sphere_renderer(ctx, -1);
	cgv::render::ref_cone_renderer(ctx, -1);
	cgv::render::ref_rectangle_renderer(ctx, -1);
}

void vr_stream_gui::init_frame(cgv::render::context& ctx)
{
	lm.ensure_texture_uptodate(ctx);
	node->ref_rectangles()->set_texture(lm.get_texture());

	if (stream_screen) 
		screen_tex_manager->update(ctx);

	if (vr_view_ptr && vr_view_ptr->get_rendered_vr_kit() != 0 && vr_view_ptr->get_rendered_eye() == 0 && vr_view_ptr->get_rendered_vr_kit() == vr_view_ptr->get_current_vr_kit()) {
		vr::vr_kit* kit_ptr = vr_view_ptr->get_current_vr_kit();
		if (kit_ptr) {
			vr::vr_camera* camera_ptr = kit_ptr->get_camera();
			if (camera_ptr && camera_ptr->get_state() == vr::CS_STARTED) {
				uint32_t width = frame_width, height = frame_height, split = frame_split;
				if (shared_texture) {
					box2 tex_range;
					if (camera_ptr->get_gl_texture_id(camera_tex_id, width, height, undistorted, &tex_range.ref_min_pnt()(0))) {
						camera_aspect = (float)width / height;
						split = camera_ptr->get_frame_split();
						switch (split) {
						case vr::CFS_VERTICAL:
							camera_aspect *= 2;
							break;
						case vr::CFS_HORIZONTAL:
							camera_aspect *= 0.5f;
							break;
						}
					}
					else
						camera_tex_id = -1;
				}
				else {
					std::vector<uint8_t> frame_data;
					if (camera_ptr->get_frame(frame_data, width, height, undistorted, max_rectangle)) {
						camera_aspect = (float)width / height;
						split = camera_ptr->get_frame_split();
						switch (split) {
						case vr::CFS_VERTICAL:
							camera_aspect *= 2;
							break;
						case vr::CFS_HORIZONTAL:
							camera_aspect *= 0.5f;
							break;
						}
						cgv::data::data_format df(width, height, cgv::type::info::TI_UINT8, cgv::data::CF_RGBA);
						cgv::data::data_view dv(&df, frame_data.data());
						if (camera_tex.is_created()) {
							if (camera_tex.get_width() != width || camera_tex.get_height() != height)
								camera_tex.destruct(ctx);
							else
								camera_tex.replace(ctx, 0, 0, dv);
						}
						if (!camera_tex.is_created())
							camera_tex.create(ctx, dv);
					}
					else if (camera_ptr->has_error())
						cgv::gui::message(camera_ptr->get_last_error());
				}
				if (frame_width != width || frame_height != height) {
					frame_width = width;
					frame_height = height;

					center_left(0) = camera_centers[2](0) / frame_width;
					center_left(1) = camera_centers[2](1) / frame_height;
					center_right(0) = camera_centers[3](0) / frame_width;
					center_right(1) = camera_centers[3](1) / frame_height;

					update_member(&frame_width);
					update_member(&frame_height);
					update_member(&center_left(0));
					update_member(&center_left(1));
					update_member(&center_right(0));
					update_member(&center_right(1));
				}
				if (split != frame_split) {
					frame_split = split;
					update_member(&frame_split);
				}
			}
		}
	}
}

void vr_stream_gui::draw(cgv::render::context& ctx)
{
	if (intersection.hit) {
		auto ri = normalize(intersection.pos_world - ray_origin);
		util::debug::line_t line;
		line.a = ray_origin;
		line.b = intersection.pos_world;
		line.col = vec4(0.0f, 1.0f, 0.0f, 1.0f);
		util::debug::draw_line(line);
		util::debug::point_t pnt;
		pnt.p = intersection.pos_world - ri * 0.01f;
		pnt.col = vec4(0.0f, 0.0f, 1.0f, 1.0f);
		util::debug::draw_point(pnt, 10.0f);
	}
	/*
	else {
		util::debug::line_t line;
		line.a = ray_origin;
		line.b = ray_origin + normalize(ray_dir) * 10.0f;
		line.col = vec4(1.0f, 0.0f, 0.0f, 1.0f);
		util::debug::draw_line(line);
	}*/

	vt_display->draw(ctx);

	if (vr_view_ptr) {
		if ((!shared_texture && camera_tex.is_created()) || (shared_texture && camera_tex_id != -1)) {
			if (vr_view_ptr->get_rendered_vr_kit() != 0 && vr_view_ptr->get_rendered_vr_kit() == vr_view_ptr->get_current_vr_kit()) {
				int eye = vr_view_ptr->get_rendered_eye();

				// compute billboard
				dvec3 vd = vr_view_ptr->get_view_dir_of_kit();
				dvec3 y = vr_view_ptr->get_view_up_dir_of_kit();
				dvec3 x = normalize(cross(vd, y));
				y = normalize(cross(x, vd));
				x *= camera_aspect * background_extent * background_distance;
				y *= background_extent * background_distance;
				vd *= background_distance;
				dvec3 eye_pos = vr_view_ptr->get_eye_of_kit(eye);
				std::vector<vec3> P;
				std::vector<vec2> T;
				P.push_back(eye_pos + vd - x - y);
				P.push_back(eye_pos + vd + x - y);
				P.push_back(eye_pos + vd - x + y);
				P.push_back(eye_pos + vd + x + y);
				double v_offset = 0.5 * (1 - eye);
				T.push_back(dvec2(0.0, 0.5 + v_offset));
				T.push_back(dvec2(1.0, 0.5 + v_offset));
				T.push_back(dvec2(0.0, v_offset));
				T.push_back(dvec2(1.0, v_offset));

				cgv::render::shader_program& prog = seethrough;
				cgv::render::attribute_array_binding::set_global_attribute_array(ctx, prog.get_position_index(), P);
				cgv::render::attribute_array_binding::set_global_attribute_array(ctx, prog.get_texcoord_index(), T);
				cgv::render::attribute_array_binding::enable_global_array(ctx, prog.get_position_index());
				cgv::render::attribute_array_binding::enable_global_array(ctx, prog.get_texcoord_index());

				GLint active_texture, texture_binding;
				if (shared_texture) {
					glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
					glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture_binding);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, camera_tex_id);
				}
				else
					camera_tex.enable(ctx, 0);
				prog.set_uniform(ctx, "texture", 0);
				prog.set_uniform(ctx, "seethrough_gamma", seethrough_gamma);
				prog.set_uniform(ctx, "use_matrix", use_matrix);

				// use of convenience function
				vr::configure_seethrough_shader_program(ctx, prog, frame_width, frame_height,
					vr_view_ptr->get_current_vr_kit(), *vr_view_ptr->get_current_vr_state(),
					0.01f, 2 * background_distance, eye, undistorted);

				/* equivalent detailed code relies on more knowledge on program parameters
				mat4 TM = vr::get_texture_transform(vr_view_ptr->get_current_vr_kit(), *vr_view_ptr->get_current_vr_state(), 0.01f, 2 * background_distance, eye, undistorted);
				prog.set_uniform(ctx, "texture_matrix", TM);

				prog.set_uniform(ctx, "extent_texcrd", extent_texcrd);
				prog.set_uniform(ctx, "frame_split", frame_split);
				prog.set_uniform(ctx, "center_left", center_left);
				prog.set_uniform(ctx, "center_right", center_right);
				prog.set_uniform(ctx, "eye", eye);
				*/
				prog.enable(ctx);
				ctx.set_color(rgba(1, 1, 1, 1));

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


				prog.disable(ctx);

				if (shared_texture) {
					glActiveTexture(active_texture);
					glBindTexture(GL_TEXTURE_2D, texture_binding);
				}
				else
					camera_tex.disable(ctx);

				cgv::render::attribute_array_binding::disable_global_array(ctx, prog.get_position_index());
				cgv::render::attribute_array_binding::disable_global_array(ctx, prog.get_texcoord_index());
			}
		}
	}
}

void vr_stream_gui::finish_draw(cgv::render::context& ctx)
{
	return;
	if ((!shared_texture && camera_tex.is_created()) || (shared_texture && camera_tex_id != -1)) {
		cgv::render::shader_program& prog = ctx.ref_default_shader_program(true);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GLint active_texture, texture_binding;
		if (shared_texture) {
			glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture_binding);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, camera_tex_id);
		}
		else
			camera_tex.enable(ctx, 0);

		prog.set_uniform(ctx, "texture", 0);
		ctx.push_modelview_matrix();
		ctx.mul_modelview_matrix(cgv::math::translate4<double>(0, 3, 0));
		prog.enable(ctx);
		ctx.set_color(rgba(1, 1, 1, 0.8f));
		ctx.tesselate_unit_square();
		prog.disable(ctx);
		if (shared_texture) {
			glActiveTexture(active_texture);
			glBindTexture(GL_TEXTURE_2D, texture_binding);
		}
		else
			camera_tex.disable(ctx);
		ctx.pop_modelview_matrix();
		glDisable(GL_BLEND);
	}
}

void vr_stream_gui::create_gui() 
{
	add_decorator("vr_stream_gui", "heading", "level=2");
	
	if (begin_tree_node("virtual display", stream_screen, false)) {
		align("\a");
		add_gui("screen orientation", reinterpret_cast<vec4&>(vt_display->ref_orientation()), "direction", "options='min=-1;max=1;ticks=true'");
		add_gui("screen position", vt_display->ref_position(), "", "options='min=-2;max=2;ticks=true'");
		add_gui("screen scale", vt_display->ref_scale(), "", "options='min=0.1;max=100;log=true;ticks=true'");
		connect_copy(add_control("stream screen", stream_screen, "toggle")->value_change, rebind(this, &vr_stream_gui::set_stream_screen));
		add_member_control(this, "switch hands", switch_left_and_right_hand, "toggle");
		add_member_control(this, "prohibit vr input", prohibit_vr_input, "toggle");
		add_member_control(this, "recalibrate virtual screen", vt_display_reconfiguring, "toggle");
		connect_copy(add_control("debug", draw_debug, "toggle")->value_change, rebind(this, &vr_stream_gui::set_draw_debug));

		align("\b");
		end_tree_node(stream_screen);
	}
	if (begin_tree_node("cameras", show_seethrough, false)) {
		align("\a");
		add_member_control(this, "show_seethrough", show_seethrough, "check");
		if (last_kit_handle) {
			add_decorator("cameras", "heading", "level=3");
			add_view("nr", nr_cameras);
			if (nr_cameras > 0) {
				connect_copy(add_button("start")->click, cgv::signal::rebind(this, &vr_stream_gui::start_camera));
				connect_copy(add_button("stop")->click, cgv::signal::rebind(this, &vr_stream_gui::stop_camera));
				add_view("frame_width", frame_width, "", "w=20", "  ");
				add_view("height", frame_height, "", "w=20", "  ");
				add_view("split", frame_split, "", "w=50");
				add_member_control(this, "undistorted", undistorted, "check");
				add_member_control(this, "shared_texture", shared_texture, "check");
				add_member_control(this, "max_rectangle", max_rectangle, "check");
				add_member_control(this, "use_matrix", use_matrix, "check");
				add_member_control(this, "gamma", seethrough_gamma, "value_slider", "min=0.1;max=10;log=true;ticks=true");
				add_member_control(this, "extent_x", extent_texcrd[0], "value_slider", "min=0.2;max=2;ticks=true");
				add_member_control(this, "extent_y", extent_texcrd[1], "value_slider", "min=0.2;max=2;ticks=true");
				add_member_control(this, "center_left_x", center_left[0], "value_slider", "min=0.2;max=0.8;ticks=true");
				add_member_control(this, "center_left_y", center_left[1], "value_slider", "min=0.2;max=0.8;ticks=true");
				add_member_control(this, "center_right_x", center_right[0], "value_slider", "min=0.2;max=0.8;ticks=true");
				add_member_control(this, "center_right_y", center_right[1], "value_slider", "min=0.2;max=0.8;ticks=true");
				add_member_control(this, "background_distance", background_distance, "value_slider", "min=0.1;max=10;log=true;ticks=true");
				add_member_control(this, "background_extent", background_extent, "value_slider", "min=0.01;max=10;log=true;ticks=true");
			}
			vr::vr_kit* kit_ptr = vr::get_vr_kit(last_kit_handle);
			const std::vector<std::pair<int, int> >* t_and_s_ptr = 0;
			if (kit_ptr)
				t_and_s_ptr = &kit_ptr->get_controller_throttles_and_sticks(0);
		}
		align("\b");
		end_tree_node(show_seethrough);
	}

	if (begin_tree_node("scene", scene, false)) {
		align("\a");
		inline_object_gui(scene);
		align("\b");
		end_tree_node(scene);
	}
	if (begin_tree_node("lab", lab, false)) {
		align("\a");
		inline_object_gui(lab);
		align("\b");
		end_tree_node(lab);
	}
	if (begin_tree_node("node", node, false)) {
		align("\a");
		inline_object_gui(node);
		align("\b");
		end_tree_node(node);
	}
	if (begin_tree_node("mesh_node", mesh_node, false)) {
		align("\a");
		inline_object_gui(mesh_node);
		align("\b");
		end_tree_node(mesh_node);
	}
	for (int i = 0; i < 2; ++i) {
		if (begin_tree_node(std::string("tool_")+cgv::utils::to_string(i), tools[i])) {
			align("\a");
			inline_object_gui(tools[i]);
			align("\b");
			end_tree_node(tools[i]);
		}
	}
}

#include <cgv/base/register.h>

cgv::base::object_registration<vr_stream_gui> vr_stream_gui_reg("vr_stream_gui");
