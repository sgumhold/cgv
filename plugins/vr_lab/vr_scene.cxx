#include "vr_scene.h"
#include <cgv/base/register.h>
#include <cgv/base/import.h>
#include <cgv/math/ftransform.h>
#include <cgv/math/pose.h>
#include <cg_vr/vr_events.h>
#include <cgv/os/cmdline_tools.h>
#include <cgv/gui/dialog.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/defines/quote.h>
#include <random>

namespace vr {

cgv::reflect::enum_reflection_traits<GroundMode> get_reflection_traits(const GroundMode& gm)
{
	return cgv::reflect::enum_reflection_traits<GroundMode>("none,boxes,terrain");
}
cgv::reflect::enum_reflection_traits<EnvironmentMode> get_reflection_traits(const EnvironmentMode& em)
{
	return cgv::reflect::enum_reflection_traits<EnvironmentMode>("empty,skybox,procedural");
}
void vr_scene::register_object(base_ptr object, const std::string& options)
{
	if (object->get_interface<cgv::nui::vr_table>())
		table = object->cast<cgv::nui::vr_table>();
	auto* foc_ptr = object->get_interface<cgv::nui::focusable>();
	if (!foc_ptr)
		return;
	std::cout << "register focusable: " << (object->get_named() ? object->get_named()->get_name() : object->get_type_name()) << std::endl;
	add_object(object);
}
void vr_scene::unregister_object(base_ptr object, const std::string& options)
{
	auto* foc_ptr = object->get_interface<cgv::nui::focusable>();
	if (!foc_ptr)
		return;
	std::cout << "unregister focusable: " << (object->get_named() ? object->get_named()->get_name() : object->get_type_name()) << std::endl;
	remove_object(object);
}
void vr_scene::set_label_border_color(const rgba& border_color)
{
	rrs.default_border_color = border_color;
	update_member(&rrs.percentual_border_width);
}
void vr_scene::set_label_border_width(float border_width)
{
	rrs.percentual_border_width = border_width;
	update_member(&rrs.percentual_border_width);
}
void vr_scene::construct_room(float w, float d, float h, float W, bool walls, bool ceiling) {
	// construct floor
	boxes.push_back(box3(vec3(-0.5f * w, -W, -0.5f * d), vec3(0.5f * w, 0, 0.5f * d)));
	box_colors.push_back(rgb(0.2f, 0.2f, 0.2f));

	if (walls) {
		// construct walls
		boxes.push_back(box3(vec3(-0.5f * w, -W, -0.5f * d - W), vec3(0.5f * w, h, -0.5f * d)));
		box_colors.push_back(rgb(0.8f, 0.5f, 0.5f));
		boxes.push_back(box3(vec3(-0.5f * w, -W, 0.5f * d), vec3(0.5f * w, h, 0.5f * d + W)));
		box_colors.push_back(rgb(0.8f, 0.5f, 0.5f));

		boxes.push_back(box3(vec3(0.5f * w, -W, -0.5f * d - W), vec3(0.5f * w + W, h, 0.5f * d + W)));
		box_colors.push_back(rgb(0.5f, 0.8f, 0.5f));

		boxes.push_back(box3(vec3(-0.5f * w, -W, -0.5f * d - W), vec3(-0.5f * w - W, h, 0.5f * d + W)));
		box_colors.push_back(rgb(0.5f, 0.8f, 0.5f));
	}
	if (ceiling) {
		// construct ceiling
		boxes.push_back(box3(vec3(-0.5f * w - W, h, -0.5f * d - W), vec3(0.5f * w + W, h + W, 0.5f * d + W)));
		box_colors.push_back(rgb(0.5f, 0.5f, 0.8f));
	}
}
void vr_scene::construct_ground(float s, float ew, float ed, float w, float d, float h) {
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0, 1);
	unsigned n = unsigned(ew / s);
	unsigned m = unsigned(ed / s);
	float ox = 0.5f * float(n) * s;
	float oz = 0.5f * float(m) * s;
	for (unsigned i = 0; i < n; ++i) {
		float x = i * s - ox;
		for (unsigned j = 0; j < m; ++j) {
			float z = j * s - oz;
			if (fabsf(x) < 0.5f * w && fabsf(x + s) < 0.5f * w && fabsf(z) < 0.5f * d && fabsf(z + s) < 0.5f * d)
				continue;
			float h = 0.2f * (std::max(abs(x) - 0.5f * w, 0.0f) + std::max(abs(z) - 0.5f * d, 0.0f)) * distribution(generator) + 0.1f;
			boxes.push_back(box3(vec3(x, 0.0f, z), vec3(x + s, h, z + s)));
			rgb color = cgv::media::color<float, cgv::media::HLS>(distribution(generator), 0.1f * distribution(generator) + 0.15f, 0.3f);
			box_colors.push_back(color);
		}
	}
}
void vr_scene::build_scene(float w, float d, float h, float W)
{
	if (draw_room) {
		construct_room(w, d, h, W, draw_walls, draw_ceiling);
	}
	if (ground_mode == GM_BOXES) {
		construct_ground(0.3f, 3 * w, 3 * d, w, d, h);
	}
}
void vr_scene::clear_scene()
{
	boxes.clear();
	box_colors.clear();
}
vr_scene::vr_scene() : lm(false)
{
	set_name("vr_scene");
	vr_view_ptr = 0;

	std::fill(valid, valid + 5, false);

	draw_controller_mode = true;
	crs.radius = 0.005f;
	crs.rounded_caps = true;
	srs.radius = 0.005f;

	draw_room = true;
	draw_walls = false;
	draw_ceiling = false;

	ground_mode = GM_BOXES;

	environment_mode = EM_EMPTY;

	terrain_style.noise_layers.emplace_back(300.0F, 15.0F);
	terrain_style.noise_layers.emplace_back(150.0F, 7.5F);
	terrain_style.noise_layers.emplace_back(80.0F, 4.0F);
	terrain_style.noise_layers.emplace_back(30.0F, 2.0F);
	terrain_style.noise_layers.emplace_back(7.5F, 0.75F);

	terrain_style.material.set_brdf_type(cgv::media::illum::BT_OREN_NAYAR);
	terrain_style.material.set_roughness(1.0f);
	terrain_style.material.set_ambient_occlusion(0.5f);
	terrain_translation = dvec3(0, -14.0, 0);
	terrain_scale = 0.1f;

	grid_width = 10;
	grid_height = 10;

	rrs.map_color_to_material = cgv::render::CM_COLOR_AND_OPACITY;
	rrs.border_mode = cgv::render::RBM_MIN;
	rrs.texture_mode = cgv::render::RTM_RED_MIX_COLOR_AND_SECONDARY_COLOR;
	rrs.illumination_mode = cgv::render::IM_OFF;

	room_width = 5;
	room_depth = 7;
	room_height = 3;
	wall_width = 0.2f;
	build_scene(room_width, room_depth, room_height, wall_width);

	pixel_scale = 0.001f;
}
bool vr_scene::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return 		
		rh.reflect_member("invert_skybox", invert_skybox) &&
		rh.reflect_member("skybox_file_names", skybox_file_names) &&
		rh.reflect_member("draw_room", draw_room) &&
		rh.reflect_member("room_width", room_width) &&
		rh.reflect_member("room_depth", room_depth) &&
		rh.reflect_member("room_height", room_height) &&
		rh.reflect_member("wall_width", wall_width) &&
		rh.reflect_member("draw_walls", draw_walls) &&
		rh.reflect_member("terrain_y_offset", terrain_translation[1]) &&
		rh.reflect_member("terrain_scale", terrain_scale) &&
		rh.reflect_member("draw_ceiling", draw_ceiling) &&
		rh.reflect_member("ground_mode", ground_mode) &&
		rh.reflect_member("ctrl_pointing_animation_duration", ctrl_pointing_animation_duration) &&
		rh.reflect_member("dispatch_mouse_spatial", dispatch_mouse_spatial) &&
		rh.reflect_member("environment_mode", environment_mode);
}
void vr_scene::on_set(void* member_ptr)
{
	if (member_ptr == &ground_mode) {
		clear_scene();
		build_scene(room_width, room_depth, room_height, wall_width);
		if (ground_mode == GM_TERRAIN && get_context()) {
			cgv::render::context& ctx = *get_context();
			static bool terrain_textures_loaded = false;
			if (ground_mode == GM_TERRAIN) {
				if (!terrain_textures_loaded) {
					terrain_style.load_default_textures(ctx);
					terrain_textures_loaded = true;
				}
				static int prev_grid_width = 0;
				static int prev_grid_height = 0;
				if (grid_width != prev_grid_width || grid_height != prev_grid_height) {
					custom_positions.clear();
					custom_indices.clear();
					static std::vector<float> quadVertices = {
						  0.0f, 0.0f, //
						  1.0f, 0.0f, //
						  1.0f, 1.0f, //
						  0.0f, 1.0f, //
					};
					for (int row = 0; row < grid_height; row++) {
						for (int col = 0; col < grid_width; col++) {
							for (int i = 0; i < static_cast<int64_t>(quadVertices.size() / 2); i++) {
								float x = (quadVertices[i * 2] + static_cast<float>(row - grid_height / 2)) * 100;
								float y = (quadVertices[i * 2 + 1] + static_cast<float>(col - grid_width / 2)) * 100;
								custom_positions.emplace_back(x, y);
							}
						}
					}

					for (int row = 0; row < grid_height; row++) {
						for (int col = 0; col < grid_width; col++) {
							const int i = (row * grid_width + col) * 4;
							custom_indices.emplace_back(i);
							custom_indices.emplace_back(i + 1);
							custom_indices.emplace_back(i + 2);
							custom_indices.emplace_back(i);
							custom_indices.emplace_back(i + 2);
							custom_indices.emplace_back(i + 3);
						}
					}
					prev_grid_width = grid_width;
					prev_grid_height = grid_height;
				}
			}
		}
	}
	if (member_ptr == &environment_mode) {
		if (environment_mode == EM_SKYBOX) {
			if (skybox_file_names.empty()) {
				std::string zip_file_path = cgv::base::find_or_download_data_file("vr_scene_skybox.zip", "cmD",
					"https://learnopengl.com/img/textures/skybox.zip", "Dcm", "vr_scene", "skyboxes", QUOTE_SYMBOL_VALUE(INPUT_DIR),
					cgv::base::user_feedback(&cgv::gui::message, &cgv::gui::question, &cgv::gui::directory_save_dialog));
				if (!zip_file_path.empty()) {
					std::string base_path = cgv::utils::file::get_path(zip_file_path);
					bool success = true;
					if (!cgv::utils::file::exists(base_path + "/skybox/right.jpg")) {
						if (!cgv::os::expand_archive(zip_file_path, base_path)) {
							cgv::gui::message("could not uncompress downloaded zip archive");
							success = false;
						}
					}
					if (success) {
						skybox_file_names = base_path + "/skybox/{right,left,bottom,top,front,back}.jpg";
						invert_skybox = true;
						update_member(&invert_skybox);
						update_member(&skybox_file_names);
					}
				}
				else {
					cgv::gui::message("download of zip archive failed");
				}
			}
		}		
	}
	if (draw_room && (member_ptr == &draw_ceiling || member_ptr == &draw_walls || member_ptr == &room_depth || member_ptr == &room_width || member_ptr == &room_height || member_ptr == &wall_width)) {
		clear_scene();
		build_scene(room_width, room_depth, room_height, wall_width);
	}
	update_member(member_ptr);
	post_redraw();
}
bool vr_scene::init(cgv::render::context& ctx)
{

//	for (size_t li = 0; li < ctx.get_nr_default_light_sources(); ++li) {
//		cgv::media::illum::light_source ls = ctx.get_default_light_source(li);
//		ls.set_local_to_eye(false);
//		ctx.set_default_light_source(li, ls);
//	}

	if (!cubemap_prog.build_program(ctx, "cubemap.glpr", true)) {
		std::cerr << "could not build cubemap program" << std::endl;
	}

	cgv::render::ref_sphere_renderer(ctx, 1);
	cgv::render::ref_cone_renderer(ctx, 1);
	cgv::render::ref_box_renderer(ctx, 1);
	cgv::render::ref_rectangle_renderer(ctx, 1);
	cgv::render::ref_terrain_renderer(ctx, 1);

	aam.init(ctx);
	cgv::gui::connect_vr_server(true);
	lm.init(ctx);
	cgv::media::font::font_ptr f = cgv::media::font::default_font(true);
	ctx.enable_font_face(f->get_font_face(cgv::media::font::FFA_BOLD), 36.0f);
	lm.set_font_face(f->get_font_face(cgv::media::font::FFA_BOLD));
	lm.set_font_size(36.0f);
	lm.set_text_color(rgba(0, 0, 0, 1));

	auto view_ptr = find_view_as_node();
	if (view_ptr) {
		// if the view points to a vr_view_interactor
		vr_view_ptr = dynamic_cast<vr_view_interactor*>(view_ptr);
		if (vr_view_ptr) {
			// configure vr event processing
			vr_view_ptr->set_event_type_flags(
				cgv::gui::VREventTypeFlags(
					cgv::gui::VRE_KEY +
					cgv::gui::VRE_ONE_AXIS +
					cgv::gui::VRE_ONE_AXIS_GENERATES_KEY +
					cgv::gui::VRE_TWO_AXES +
					cgv::gui::VRE_TWO_AXES_GENERATES_DPAD +
					cgv::gui::VRE_POSE
				));
			// vr_view_ptr->enable_vr_event_debugging(false);
			// configure vr rendering
			// vr_view_ptr->draw_action_zone(false);
			vr_view_ptr->draw_vr_kits(true);
			// vr_view_ptr->enable_blit_vr_views(true);
			// vr_view_ptr->set_blit_vr_view_width(200);
		}
	}
	return true;
}
void vr_scene::init_frame(cgv::render::context& ctx)
{
	if (environment_mode == EM_SKYBOX) {
		static std::string last_file_names;
		if (skybox_file_names != last_file_names) {
			skybox.create_from_images(ctx, skybox_file_names);
			last_file_names = skybox_file_names;
		}
	}
	mat34 ID; ID.identity();
	pose[0] = pose[1] = ID;
	valid[0] = valid[1] = true, true;
	valid[2] = valid[3] = valid[4] = false;
	// update table pose
	if (table) {
		mat4 M = table->get_transform();
		pose[CS_TABLE] = mat34(4, 4, &M(0, 0));
	}
	else
		cgv::math::pose_position(pose[CS_TABLE]) = vec3(0.0f, 0.7f, 0.0f);
	// extract poses from tracked vr devices
	if (vr_view_ptr) {
		const auto* cs = vr_view_ptr->get_current_vr_state();
		if (cs) {
			valid[CS_HEAD] = cs->hmd.status == vr::VRS_TRACKED;
			valid[CS_LEFT_CONTROLLER] = cs->controller[0].status == vr::VRS_TRACKED;
			valid[CS_RIGHT_CONTROLLER] = cs->controller[1].status == vr::VRS_TRACKED;
			if (valid[CS_HEAD])
				pose[CS_HEAD] = reinterpret_cast<const mat34&>(vr_view_ptr->get_current_vr_state()->hmd.pose[0]);
			if (valid[CS_LEFT_CONTROLLER])
				pose[CS_LEFT_CONTROLLER] = reinterpret_cast<const mat34&>(vr_view_ptr->get_current_vr_state()->controller[0].pose[0]);
			if (valid[CS_RIGHT_CONTROLLER])
				pose[CS_RIGHT_CONTROLLER] = reinterpret_cast<const mat34&>(vr_view_ptr->get_current_vr_state()->controller[1].pose[0]);
		}
	}
}
void vr_scene::clear(cgv::render::context& ctx)
{
	cgv::render::ref_sphere_renderer(ctx, -1);
	cgv::render::ref_box_renderer(ctx, -1);
	cgv::render::ref_cone_renderer(ctx, -1);
	cgv::render::ref_terrain_renderer(ctx, -1);
	aam.destruct(ctx);
	lm.destruct(ctx);
	cgv::render::ref_rectangle_renderer(ctx, -1);
}
void vr_scene::draw(cgv::render::context& ctx)
{
	set_modelview_projection_window_matrix(ctx);

	bool repack = lm.is_packing_outofdate();
	lm.ensure_texture_uptodate(ctx);
	if (repack) {
		for (uint32_t li = 0; li < label_texture_ranges.size(); ++li)
			label_texture_ranges[li] = lm.get_texcoord_range(li);
	}
	if ((ground_mode == GM_BOXES) || draw_room) {
		// activate render styles
		auto& br = cgv::render::ref_box_renderer(ctx);
		br.set_render_style(box_style);

		// draw static part
		br.set_box_array(ctx, boxes);
		br.set_color_array(ctx, box_colors);
		br.render(ctx, 0, (GLsizei)boxes.size());
	}
	if (ground_mode == GM_TERRAIN && !custom_indices.empty()) {
		ctx.push_modelview_matrix();
		ctx.mul_modelview_matrix(cgv::math::scale4<double>(dvec3(terrain_scale))*cgv::math::translate4<double>(terrain_translation));
			auto& tr = cgv::render::ref_terrain_renderer(ctx);
			tr.set_render_style(terrain_style);
			tr.set_position_array(ctx, custom_positions);
			tr.set_indices(ctx, custom_indices);
			tr.render(ctx, 0, custom_indices.size());
		ctx.pop_modelview_matrix();
	}
	if (environment_mode == EM_SKYBOX && skybox.is_created()) {
		// lastly render the environment
		GLint prev_depth_func;
		glGetIntegerv(GL_DEPTH_FUNC, &prev_depth_func);
		cubemap_prog.enable(ctx);
			cubemap_prog.set_uniform(ctx, "invert", invert_skybox);
			cubemap_prog.set_uniform(ctx, "depth_value", 1.0f);
			glDepthFunc(GL_LEQUAL);
				skybox.enable(ctx, 0);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				skybox.disable(ctx);
			glDepthFunc(prev_depth_func);
		cubemap_prog.disable(ctx);
	}
	if (draw_controller_mode) {
		auto* state_ptr = vr_view_ptr->get_current_vr_state();
		if (state_ptr) {
			sphere_positions.clear();
			sphere_colors.clear();
			cone_positions.clear();
			cone_colors.clear();
			double time = cgv::gui::trigger::get_current_time();
			for (int ci = 0; ci < 2; ++ci) {
				vec3 ro, rd;
				state_ptr->controller[ci].put_ray(ro, rd);
				if (ctrl_infos[ci].grabbing) {
					sphere_positions.push_back(ro + max_grabbing_distance * rd);
					sphere_colors.push_back(ctrl_infos[ci].color);
				}
				double dt = time - ctrl_infos[ci].toggle_time;
				if (ctrl_infos[ci].pointing || dt < ctrl_pointing_animation_duration) {
					float begin = ctrl_infos[ci].grabbing ? min_pointing_distance : 0.0f;
					float end = max_pointing_distance;
					if (dt < ctrl_pointing_animation_duration) {
						float lambda = float(dt / ctrl_pointing_animation_duration);
						if (!ctrl_infos[ci].pointing)
							lambda = 1.0f - lambda;
						end = begin + lambda * (end - begin);
					}
					auto iter = dis_info_hid_map.find({ cgv::nui::hid_category::controller, vr_view_ptr->get_current_vr_kit()->get_handle(), int16_t(ci) });
					if (iter != dis_info_hid_map.end()) {
						if (iter->second) {
							if (iter->second->mode == cgv::nui::dispatch_mode::pointing) {
								auto* idi_ptr = reinterpret_cast<cgv::nui::intersection_dispatch_info*>(iter->second);
								if (idi_ptr->ray_param < std::numeric_limits<float>::max())
									end = std::min(end, begin + idi_ptr->ray_param);
							}
						}
					}
					cone_positions.push_back(ro + begin * rd);
					cone_positions.push_back(ro + end * rd);
					cone_colors.push_back(ctrl_infos[ci].color);
					cone_colors.push_back(ctrl_infos[ci].color);
				}
			}
		}
		if (!cone_positions.empty()) {
			auto& cr = cgv::render::ref_cone_renderer(ctx);
			cr.set_render_style(crs);
			cr.set_position_array(ctx, cone_positions);
			cr.set_color_array(ctx, cone_colors);
			cr.render(ctx, 0, cone_positions.size());
		}
		if (!sphere_positions.empty()) {
			auto& sr = cgv::render::ref_sphere_renderer(ctx);
			sr.set_render_style(srs);
			sr.set_position_array(ctx, sphere_positions);
			sr.set_color_array(ctx, sphere_colors);
			sr.render(ctx, 0, sphere_positions.size());
		}
	}
}
void vr_scene::finish_frame(cgv::render::context& ctx)
{
	// compute label poses in lab coordinate system
	std::vector<vec3> P;
	std::vector<quat> Q;
	std::vector<vec2> E;
	std::vector<vec4> T;
	std::vector<rgba> C;
	// set poses of visible labels in valid coordinate systems
	for (uint32_t li = 0; li < label_coord_systems.size(); ++li) {
		if (label_visibilities[li] == 0 || !valid[label_coord_systems[li]])
			continue;
		mat34 label_pose = cgv::math::pose_construct(label_orientations[li], label_positions[li]);
		cgv::math::pose_transform(pose[label_coord_systems[li]], label_pose);
		P.push_back(cgv::math::pose_position(label_pose));
		mat3 L = cgv::math::pose_orientation(label_pose);
		float s = L.frobenius_norm() / sqrt(3.0f);
		Q.push_back(quat((1/s)*L));
		E.push_back(s*label_extents[li]);
		T.push_back(label_texture_ranges[li]);
		C.push_back(lm.get_label(li).background_color);
	}
	// draw labels
	if (!P.empty()) {
		GLboolean blend = glIsEnabled(GL_BLEND); glEnable(GL_BLEND);
		GLenum blend_src, blend_dst, depth;
		glGetIntegerv(GL_BLEND_DST, reinterpret_cast<GLint*>(&blend_dst));
		glGetIntegerv(GL_BLEND_SRC, reinterpret_cast<GLint*>(&blend_src));
		glGetIntegerv(GL_DEPTH_FUNC, reinterpret_cast<GLint*>(&depth));

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		auto& rr = cgv::render::ref_rectangle_renderer(ctx);
		rr.set_render_style(rrs);
		rr.enable_attribute_array_manager(ctx, aam);
		rr.set_position_array(ctx, P);
		rr.set_rotation_array(ctx, Q);
		rr.set_extent_array(ctx, E);
		rr.set_texcoord_array(ctx, T);
		rr.set_color_array(ctx, C);
		rr.set_secondary_color(ctx, lm.get_text_color());
		lm.get_texture()->enable(ctx);
		rr.render(ctx, 0, P.size());
		lm.get_texture()->disable(ctx);
		rr.disable_attribute_array_manager(ctx, aam);

		if (!blend)
			glDisable(GL_BLEND);
		glBlendFunc(blend_src, blend_dst);
	}
}
void vr_scene::stream_help(std::ostream& os)
{
	os << "vr_scene: <?> shows focus attachments, <C-M> toggles dispatch_mouse_spatial" << std::endl;
}
void vr_scene::construct_hit_geometry()
{
	cone_positions.clear();
	cone_colors.clear();
	rgb nml_color(0.5f, 0.5f, 0.5f);
	for (const auto& di : dis_info_hid_map) {
		if (!di.second)
			continue;
		if (di.second->hid_id.category != cgv::nui::hid_category::controller)
			continue;
		if (di.second->mode == cgv::nui::dispatch_mode::pointing) {
			const auto* idi_ptr = reinterpret_cast<const cgv::nui::intersection_dispatch_info*>(di.second);			
			cone_positions.push_back(idi_ptr->ray_origin);
			cone_positions.push_back(idi_ptr->hit_point);
			rgb color = rgb(1, 1, 0);
			cone_colors.push_back(color);
			cone_colors.push_back(color);
		}
		else if (di.second->mode == cgv::nui::dispatch_mode::proximity) {
			const auto* pdi_ptr = reinterpret_cast<const cgv::nui::proximity_dispatch_info*>(di.second);
			cone_positions.push_back(pdi_ptr->query_point);
			cone_positions.push_back(pdi_ptr->hit_point);
			rgb color = rgb(1, 1, 1);
			cone_colors.push_back(color);
			cone_colors.push_back(color);
		}
	}
}
void vr_scene::check_for_detach(int ci, const cgv::gui::event& e)
{
	if (ctrl_infos[ci].grabbing || ctrl_infos[ci].pointing)
		return;
	void* vr_kit_handle = vr_view_ptr->get_current_vr_kit()->get_handle();
	cgv::nui::hid_identifier hid_id = { cgv::nui::hid_category::controller, vr_kit_handle, int16_t(ci) };
	auto iter = focus_hid_map.find(hid_id);
	if (iter != focus_hid_map.end()) {
		iter->second.object->get_interface<cgv::nui::focusable>()->focus_change(
			cgv::nui::focus_change_action::detach, cgv::nui::refocus_action::none,
			{ iter->first, iter->second.config }, e, { hid_id, cgv::nui::dispatch_mode::none });
		std::cout << "detaching hid: " << iter->first << " -> " << iter->second.config << std::endl;
		focus_hid_map.erase(iter);
	}
	cgv::nui::kit_identifier kit_id = { cgv::nui::kit_category::vr, vr_kit_handle };
	auto jter = focus_kit_map.find(kit_id);
	if (jter != focus_kit_map.end()) {
		jter->second.object->get_interface<cgv::nui::focusable>()->focus_change(
			cgv::nui::focus_change_action::detach, cgv::nui::refocus_action::none,
			{ jter->first, jter->second.config }, e, { hid_id, cgv::nui::dispatch_mode::none });
		std::cout << "detaching kit: " << jter->first << " -> " << jter->second.config << std::endl;
		focus_kit_map.erase(jter);
	}
}
bool vr_scene::handle(cgv::gui::event& e)
{
	if ((e.get_flags() & cgv::gui::EF_VR) == 0 && e.get_kind() == cgv::gui::EID_KEY) {
		const auto& ke = reinterpret_cast<const cgv::gui::key_event&>(e);
		if (ke.get_action() == cgv::gui::KA_PRESS && ke.get_char() == '?') {
			std::cout << "hid attachments:" << std::endl;
			for (auto& ha : focus_hid_map)
				std::cout << ha.first << "|" << ha.second.object->get_type_name() << ":" << ha.second.config << std::endl;
			std::cout << "kit attachments:" << std::endl;
			for (auto& ka : focus_kit_map)
				std::cout << ka.first << "|" << ka.second.object->get_type_name() << ":" << ka.second.config << std::endl;
			return true;
		}
		if (ke.get_action() == cgv::gui::KA_PRESS && ke.get_modifiers() == cgv::gui::EM_CTRL && ke.get_key() == 'M') {
			dispatch_mouse_spatial = !dispatch_mouse_spatial;
			on_set(&dispatch_mouse_spatial);
		}
	}
	if ((e.get_flags() & cgv::gui::EF_VR) != 0 && e.get_kind() == cgv::gui::EID_KEY) {
		const auto& vrke = reinterpret_cast<const cgv::gui::vr_key_event&>(e);
		int ci = vrke.get_controller_index();
		if (vrke.get_action() == cgv::gui::KA_PRESS) {
			double dt = e.get_time() - ctrl_infos[ci].toggle_time;
			switch (vrke.get_key()) {
			case vr::VR_DPAD_UP:
				ctrl_infos[ci].pointing = !ctrl_infos[ci].pointing;
				ctrl_infos[ci].toggle_time = e.get_time();
				if (dt < ctrl_pointing_animation_duration)
					ctrl_infos[ci].toggle_time -= (ctrl_pointing_animation_duration-dt);
				update_member(&ctrl_infos[ci].pointing);
				check_for_detach(ci, e);
				return true;
			case vr::VR_DPAD_DOWN:
				ctrl_infos[ci].grabbing = !ctrl_infos[ci].grabbing;
				update_member(&ctrl_infos[ci].grabbing);
				return true;
			}
		}
	}
	cgv::nui::dispatch_report report;
	bool ret = dispatch(e, &report);
	if (report.action != cgv::nui::refocus_action::none)
		grab_focus();
	return ret;
}
void vr_scene::create_gui()
{
	add_decorator("vr_scene", "heading");
	if (begin_tree_node("hids", ctrl_infos)) {
		align("\a");
		add_member_control(this, "dispatch_mouse_spatial", dispatch_mouse_spatial, "toggle");
		add_member_control(this, "ctrl_pointing_animation_duration", ctrl_pointing_animation_duration, "value_slider", "min=0;max=2;ticks=true");
		add_member_control(this, "draw_controller_mode", draw_controller_mode, "check");
		add_member_control(this, " left ctrl grabs", ctrl_infos[0].grabbing, "check");
		add_member_control(this, " left ctrl point", ctrl_infos[0].pointing, "check");
		add_member_control(this, " left ctrl color", ctrl_infos[0].color);
		add_member_control(this, "right ctrl grabs", ctrl_infos[1].grabbing, "check");
		add_member_control(this, "right ctrl point", ctrl_infos[1].pointing, "check");
		add_member_control(this, "right ctrl color", ctrl_infos[1].color);
		add_member_control(this, "intersection_bias", intersection_bias, "value_slider", "min=0.1;max=10;log=true;ticks=true");
		add_member_control(this, "max_grabbing_distance", max_grabbing_distance, "value_slider", "min=0.001;max=0.1;log=true;ticks=true");
		add_member_control(this, "min_pointing_distance", max_pointing_distance, "value_slider", "min=0.01;max=3;log=true;ticks=true");
		add_member_control(this, "max_pointing_distance", max_pointing_distance, "value_slider", "min=0.1;max=10;log=true;ticks=true");
		align("\b");
		end_tree_node(ctrl_infos);
	}
	if (begin_tree_node("environment", environment_mode)) {
		align("\a");
		add_member_control(this, "mode", environment_mode, "dropdown", "enums='empty,skybox,procedural'");
		add_member_control(this, "invert_skybox", invert_skybox, "check");
		align("\b");
		end_tree_node(environment_mode);
	}
	if (begin_tree_node("ground", ground_mode)) {
		align("\a");
		add_member_control(this, "mode", ground_mode, "dropdown", "enums='none,boxes,terrain'");
		if (begin_tree_node("Terrain Settings", grid_width, false, "level=2")) {
			align("\a");
			add_member_control(this, "grid width", grid_width, "value_slider", "min=1;max=100;log=true;ticks=true");
			add_member_control(this, "grid height", grid_height, "value_slider", "min=1;max=100;log=true;ticks=true");
			add_member_control(this, "scale", terrain_scale, "value_slider", "min=0.001;max=10;log=true;ticks=true");
			add_gui("translation", terrain_translation, "",
				"long_label=true;main_label='first';options='min=-100;max=100;ticks=true'");
			align("\b");
			end_tree_node(grid_width);
		}
		align("\b");
		end_tree_node(ground_mode);
	}
	if (begin_tree_node("room", boxes)) {
		align("\a");
		add_member_control(this, "draw room", draw_room, "check");
		add_member_control(this, "draw walls", draw_walls, "check");
		add_member_control(this, "draw ceiling", draw_ceiling, "check");
		add_member_control(this, "width", room_width, "value_slider", "min=0.1;max=20.0;ticks=true");
		add_member_control(this, "depth", room_depth, "value_slider", "min=0.1;max=20.0;ticks=true");
		add_member_control(this, "height", room_height, "value_slider", "min=0.1;max=10.0;ticks=true");
		add_member_control(this, "wall width", wall_width, "value_slider", "min=0.1;max=2.0;ticks=true");
		align("\b");
		end_tree_node(boxes);
	}
	if (begin_tree_node("labels", lm)) {
		align("\a");
		for (size_t i = 0; i < label_positions.size(); ++i) {
			if (begin_tree_node(std::string("label ") + cgv::utils::to_string(i), label_positions[i])) {
				align("\a");
				add_member_control(this, "visible", (bool&)label_visibilities[i], "toggle");
				add_member_control(this, "coordinate_system", (cgv::type::DummyEnum&)label_coord_systems[i], "dropdown", "enums='lab,table,head,left controller,right controller'");
				add_gui("position", label_positions[i], "vector", "gui_type='value_slider';options='min=-2;max=2;ticks=true'");
				add_gui("orientation", (vec4&)label_orientations[i], "direction", "gui_type='value_slider';options='min=-1;max=1;ticks=true'");
				add_gui("extent", label_extents[i], "vector", "gui_type='value_slider';options='min=0;max=1;ticks=true'");
				align("\b");
				end_tree_node(label_positions[i]);
			}
		}
		align("\b");
		end_tree_node(lm);
	}
	if (begin_tree_node("styles", draw_room)) {
		if (begin_tree_node("rectangles", rrs)) {
			align("\a");
			add_gui("rrs", rrs);
			align("\b");
			end_tree_node(rrs);
		}
		if (begin_tree_node("boxes", box_style)) {
			align("\a");
			add_gui("box_style", box_style);
			align("\b");
			end_tree_node(box_style);
		}
		if (begin_tree_node("cones", crs)) {
			align("\a");
			add_gui("cone_style", crs);
			align("\b");
			end_tree_node(crs);
		}
		if (begin_tree_node("terrain", terrain_style)) {
			align("\a");
			add_gui("terrain_render_style", terrain_style);
			align("\b");
			end_tree_node(terrain_style);
		}
		align("\b");
		end_tree_node(draw_room);
	}
}

}

#include <cgv/base/register.h>
#include <cg_nui/vr_screen.h>

cgv::base::object_registration<vr::vr_scene> vr_scene_reg("vr_scene");
cgv::base::object_registration_1<cgv::nui::vr_screen, std::string> vr_screen_reg("vr_screen");
cgv::base::object_registration<cgv::nui::vr_table> vr_table_reg("vr_table");
