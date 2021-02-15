#include "multi_view_interactor.h"
#include <cgv/math/ftransform.h>
#include <cgv/math/pose.h>
#include <cgv/gui/key_event.h>
#include <cgv_reflect_types/math/fvec.h>
#include <cgv_reflect_types/math/quaternion.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/gl/gl_tools.h>
#include <cgv_gl/rectangle_renderer.h>

///
multi_view_interactor::multi_view_interactor(const char* name) : vr_view_interactor(name)
{
	last_kit_handle = 0;
	set_screen_orientation(quat(1, 0, 0, 0));
	screen_pose.set_col(3, vec3(0, 1, 0));
	pixel_size = vec2(0.001f, 0.001f);
	pixel_scale = 0.001f;
	screen_width = 1920;
	screen_height = 1080;
	show_screen = true;
	show_eyes = false;
	debug_display_index = -1;
	debug_eye = 0;
	adapt_aspect_ratio_to_stereo_mode = false;
	show_probe = false;
	debug_probe = vec3(0.0f);
	multi_view_mode = MVM_DEBUG;
	rrs.illumination_mode = cgv::render::IM_OFF;
}

///
void multi_view_interactor::on_device_change(void* handle, bool attach)
{
	vr_view_interactor::on_device_change(handle, attach);
	if (attach) {
		last_kit_handle = handle;
	}
}

void multi_view_interactor::add_player(int ci)
{
	if (last_kit_handle == 0) {
		std::cerr << "add_controller_as_player ignored as no vr_kit is attached" << std::endl;
		return;
	}
	displays.push_back(new vr::gl_vr_display(screen_width, screen_height, 4));
	head_tracking_info hti;
	hti.kit_handle = last_kit_handle;
	hti.controller_index = ci;
	hti.local_eye_position[0] = vec3( 0.035f, -0.06f, -0.07f);
	hti.local_eye_position[1] = vec3(-0.035f, -0.06f, -0.07f);
	head_trackers.push_back(hti);
	new_displays.push_back(displays.back());
}

/// return the type name 
std::string multi_view_interactor::get_type_name() const
{
	return "multi_view_interactor";
}

/// transform to coordinate system of screen with [0,0,0] in center and corners [+-aspect,+-1,0]; z is signed distance to screen in world unites (typically meters) 
multi_view_interactor::vec3 multi_view_interactor::transform_world_to_screen(const vec3& p) const
{
	vec3 p_screen = (p - get_screen_center()) * get_screen_orientation();
	p_screen[0] /= 0.5f * screen_width * pixel_size[0];
	p_screen[1] /= 0.5f * screen_height * pixel_size[1];
	return p_screen;
}
/// transform from coordinate system of screen with [0,0,0] in center and corners [+-aspect,+-1,0]; z is signed distance to screen in world unites (typically meters) 
multi_view_interactor::vec3 multi_view_interactor::transform_screen_to_world(const vec3& p_screen) const
{
	vec3 p_s = p_screen;
	p_s[0] *= 0.5f * screen_width * pixel_size[0];
	p_s[1] *= 0.5f * screen_height * pixel_size[1];
	return get_screen_orientation() * p_s + get_screen_center();
}

/// access to 4x4 matrix in column major format for perspective transformation from eye (0..left, 1..right)
void multi_view_interactor::put_projection_matrix(unsigned user_index, int eye, float z_near, float z_far, float* projection_matrix, const float* tracker_pose) const
{
	vec3 eye_world = get_eye_position_world(user_index, eye, *reinterpret_cast<const mat34*>(tracker_pose));
	vec3 eye_screen = (eye_world - get_screen_center()) * get_screen_orientation();
	float scale = z_near / abs(eye_screen(2));
	float width_world = screen_width * pixel_size[0];
	float eye_screen_x = eye_screen(0);
	if (eye_screen(2) < 0)
		eye_screen_x = -eye_screen_x;
	float l = scale * (-0.5f * width_world - eye_screen_x);
	float r = scale * (+0.5f * width_world - eye_screen_x);
	float height_world = screen_height * pixel_size[1];
	float b = scale * (-0.5f * height_world - eye_screen(1));
	float t = scale * (+0.5f * height_world - eye_screen(1));
	reinterpret_cast<mat4&>(*projection_matrix) = cgv::math::frustum4<float>(l, r, b, t, z_near, z_far);
}

void multi_view_interactor::put_world_to_eye_transform(unsigned user_index, int eye, const float* tracker_pose, float* modelview_matrix, vec3* eye_world_ptr, vec3* view_dir_ptr) const
{
	vec3 eye_world = get_eye_position_world(user_index, eye, *reinterpret_cast<const mat34*>(tracker_pose));
	if (eye_world_ptr)
		*eye_world_ptr = eye_world;
	mat3 R = get_screen_orientation();
	vec3 eye_screen = (eye_world - get_screen_center()) * get_screen_orientation();
	if (eye_screen(2) < 0) {
		R.set_col(0, -R.col(0));
		R.set_col(2, -R.col(2));
	}
	if (view_dir_ptr)
		*view_dir_ptr = -R.col(2);
	R.transpose();
	mat4& T = reinterpret_cast<mat4&>(*modelview_matrix);
	T.set_col(0, vec4(R.col(0), 0));
	T.set_col(1, vec4(R.col(1), 0));
	T.set_col(2, vec4(R.col(2), 0));
	T.set_col(3, vec4(-R * eye_world, 1));
}

/// 
bool multi_view_interactor::init(cgv::render::context& ctx)
{
	cgv::render::ref_box_renderer(ctx, 1);
	cgv::render::ref_rectangle_renderer(ctx, 1);
	return vr_view_interactor::init(ctx);
}

/// 
void multi_view_interactor::clear(cgv::render::context& ctx)
{
	cgv::render::ref_box_renderer(ctx, -1);
	cgv::render::ref_rectangle_renderer(ctx, -1);
	vr_view_interactor::clear(ctx);
}

const vr::vr_trackable_state* multi_view_interactor::get_trackable_state(int display_index) const
{
	auto iter = find(kits.begin(), kits.end(), head_trackers[display_index].kit_handle);
	if (iter == kits.end())
		return 0;
	int ci = head_trackers[display_index].controller_index;
	if (ci == -1)
		return &kit_states[iter - kits.begin()].hmd;
	else
		return &kit_states[iter - kits.begin()].controller[ci];
}

/// this method is called in one pass over all drawables before the draw method
void multi_view_interactor::init_frame(cgv::render::context& ctx)
{
	while (!new_displays.empty()) {
		new_displays.back()->init_fbos();
		new_displays.pop_back();
		if (find_control(debug_display_index)) 
			find_control(debug_display_index)->set("max", int(displays.size())-1);
	}
	if (ctx.get_render_pass() == cgv::render::RP_MAIN) {
		// update current vr kits
		configure_kits();
		// perform rendering from the vr kits
		if (kits.size() > 0) {
			// query vr state
			query_vr_states();
			// render all user views
			cgv::render::RenderPassFlags rpf = ctx.get_render_pass_flags();
			// render all user views and take last eye into main render pass if no separate view is rendered
			for (rendered_display_index = 0; rendered_display_index<int(displays.size()); ++rendered_display_index) {
				rendered_display_ptr = displays[rendered_display_index];
				if (!rendered_display_ptr)
					continue;
				auto* ts_ptr = get_trackable_state(rendered_display_index);
				if (!ts_ptr || ts_ptr->status == vr::VRS_DETACHED)
					continue;

				void* fbo_handle;
				ivec4 cgv_viewport;
				for (rendered_eye = 0; rendered_eye < 2; ) {
					rendered_display_ptr->enable_fbo(rendered_eye);
					ctx.announce_external_frame_buffer_change(fbo_handle);
					ctx.announce_external_viewport_change(cgv_viewport);
					ctx.render_pass(cgv::render::RP_USER_DEFINED, cgv::render::RenderPassFlags(rpf & ~cgv::render::RPF_HANDLE_SCREEN_SHOT), this);
					rendered_display_ptr->disable_fbo(rendered_eye);
					ctx.recover_from_external_viewport_change(cgv_viewport);
					ctx.recover_from_external_frame_buffer_change(fbo_handle);
					++rendered_eye;
					if (!separate_view && rendered_display_index == (int)displays.size() - 1)
						break;
				}
			}
			if (separate_view) {
				rendered_display_ptr = 0;
				rendered_display_index = -1;
			}
		}
	}
	// set model view projection matrices for currently rendered eye of rendered vr kit
	if (rendered_display_ptr) {
		mat4 P, M;
		vec3 eye_world, view_dir;
		const auto* ts_ptr = get_trackable_state(rendered_display_index);
		if (ts_ptr) {
			put_world_to_eye_transform(rendered_display_index, rendered_eye, ts_ptr->pose, &M[0, 0], &eye_world, &view_dir);
			double z_near_derived, z_far_derived;
			compute_clipping_planes(eye_world, view_dir, z_near_derived, z_far_derived, clip_relative_to_extent);
			put_projection_matrix(rendered_display_index, rendered_eye, (float)z_near_derived, (float)z_far_derived, &P[0, 0], ts_ptr->pose);
			ctx.set_projection_matrix(P);
			ctx.set_modelview_matrix(M);
		}
	}
	else {
		// use standard rendering for separate view or if no vr kit is available
		if (kits.empty() || separate_view)
			stereo_view_interactor::init_frame(ctx);
	}
}

///
bool multi_view_interactor::self_reflect(cgv::reflect::reflection_handler& srh)
{
	return
		srh.reflect_base<vr_view_interactor>(*static_cast<vr_view_interactor*>(this)) &&
		srh.reflect_member("add_controller_as_player", add_controller_as_player) &&
		srh.reflect_member("debug_eye", debug_eye) &&
		srh.reflect_member("multi_view_mode", multi_view_mode) &&
		srh.reflect_member("debug_display_index", debug_display_index) &&
		srh.reflect_member("show_eyes", show_eyes) &&
		srh.reflect_member("pixel_scale", pixel_scale) &&
		srh.reflect_member("show_screen", show_screen) &&
		srh.reflect_member("screen_orientation", screen_orientation) &&
		srh.reflect_member("show_probe", show_probe) &&
		srh.reflect_member("debug_probe", debug_probe) &&
		srh.reflect_member("screen_width", screen_width) &&
		srh.reflect_member("screen_height", screen_height) &&
		srh.reflect_member("screen_center", reinterpret_cast<vec3&>(screen_pose(0, 3)));
}

void multi_view_interactor::on_set(void* member_ptr)
{
	if (member_ptr == &pixel_scale) {
		pixel_size = vec2(pixel_scale);
	}
	if (member_ptr >= &screen_orientation && member_ptr < &screen_orientation + 1) {
		set_screen_orientation(screen_orientation);
	}
	if (member_ptr == &add_controller_as_player) {
		add_player(add_controller_as_player);
	}
	vr_view_interactor::on_set(member_ptr);
}

void multi_view_interactor::draw(cgv::render::context& ctx)
{
	static int has_stereo = 0;
	if (ctx.get_render_pass() == cgv::render::RP_STEREO)
		has_stereo = 2;
	else
		has_stereo = has_stereo > 0 ? has_stereo - 1 : 0;
	int eye = debug_eye;
	if (has_stereo > 0)
		eye = ctx.get_render_pass() == cgv::render::RP_STEREO ? 0 : 1;

	// draw spheres
	std::vector<vec3> P;
	std::vector<rgb> C;
	std::vector<float> R;
	if (show_eyes) {
		for (int display_index = 0; display_index < (int)displays.size(); ++display_index) {
			auto* ts_ptr = get_trackable_state(display_index);
			if (ts_ptr->status == vr::VRS_DETACHED)
				continue;
			P.push_back(get_eye_position_world(display_index, 0, reinterpret_cast<const mat34&>(*ts_ptr->pose)));
			C.push_back(rgb(1, 0, 0));
			R.push_back(0.03f);
			P.push_back(get_eye_position_world(display_index, 1, reinterpret_cast<const mat34&>(*ts_ptr->pose)));
			C.push_back(rgb(0, 1, 1));
			R.push_back(0.03f);
		}
	}
	if (show_screen) {
		P.push_back(get_screen_center() + 0.05f * get_screen_orientation().col(2));
		R.push_back(0.05f);
		C.push_back(rgb(0.8f, 0.5f, 0.5f));
	}
	if (show_probe && debug_display_index >= 0 && debug_display_index < (int)displays.size()) {
		for (int i = 0; i < 50; ++i) {
			float lambda = 0.1f * i;
			P.push_back((1.0f - lambda) * get_eye_position_world(debug_display_index, eye, reinterpret_cast<const mat34&>(get_trackable_state(debug_display_index)->pose))
				+ lambda * debug_probe);
			if (i == 10) {
				R.push_back(0.04f);
				C.push_back(rgb(0.5f, 1, 0.5f));
			}
			else {
				R.push_back(0.01f);
				C.push_back(rgb(0, 1, 0));
			}
		}
	}
	if (!P.empty()) {
		auto& sr = cgv::render::ref_sphere_renderer(ctx);
		sr.set_render_style(srs);
		sr.set_position_array(ctx, P);
		sr.set_radius_array(ctx, R);
		sr.set_color_array(ctx, C);
		sr.render(ctx, 0, (GLuint)P.size());
	}
	if (show_screen && ctx.get_render_pass() != cgv::render::RP_USER_DEFINED) {
		auto& rr = cgv::render::ref_rectangle_renderer(ctx, 1);
		rr.set_render_style(rrs);
		vec3 screen_center = get_screen_center();
		quat screen_orientation(get_screen_orientation());
		vec2 extent(screen_width * pixel_size[0], screen_height * pixel_size[1]);
		rgb screen_color(0.5f, 0.5f, 0.5f);
		vec4 tex_range(0, 0, 1, 1);

		rr.set_position_array(ctx, &screen_center, 1);
		rr.set_extent_array(ctx, &extent, 1);
		rr.set_color_array(ctx, &screen_color, 1);
		rr.set_rotation_array(ctx, &screen_orientation, 1);
		if (debug_display_index >= 0 && debug_display_index < (int)displays.size()) {
			vec3 eye_screen = (get_eye_position_world(debug_display_index, eye, reinterpret_cast<const mat34&>(get_trackable_state(debug_display_index)->pose)) - get_screen_center())* get_screen_orientation();
			if (eye_screen(2) < 0)
				tex_range = vec4(1, 0, 0, 1);
			rr.set_texcoord_array(ctx, &tex_range, 1);
			displays[debug_display_index]->bind_texture(eye);
		}
		if (rr.validate_and_enable(ctx)) {
			rr.ref_prog().set_uniform(ctx, "gamma", 1.0f);
			rr.draw(ctx, 0, 1);
			rr.disable(ctx);
		}
	}
	vr_view_interactor::draw(ctx);
}

void multi_view_interactor::after_finish(cgv::render::context& ctx)
{
	stereo_view_interactor::after_finish(ctx);
	if (ctx.get_render_pass() == cgv::render::RP_MAIN) {
		if (rendered_display_ptr) {
			rendered_display_ptr->disable_fbo(rendered_eye);
				ctx.recover_from_external_frame_buffer_change(fbo_handle);
				ctx.recover_from_external_viewport_change(cgv_viewport);
				rendered_eye = 0;
				rendered_display_ptr = 0;
				rendered_display_index = -1;
		}
	}
	if (ctx.get_render_pass() == cgv::render::RP_MAIN ||
		ctx.get_render_pass() == cgv::render::RP_STEREO) {


		/// here we handle
		/// left eye if we are in RP_STEREO render pass and
		/// right eye in RP_MAIN render pass

		switch (multi_view_mode) {
		case MVM_DEBUG:
		{
			int x_offset = 0;
			int y_offset = 0;
			if (stereo_enabled) {
				if (ctx.get_render_pass() == cgv::render::RP_MAIN && stereo_mode == GLSU_SPLIT_VERTICALLY)
					x_offset = ctx.get_width() / 2;
				if (ctx.get_render_pass() == cgv::render::RP_STEREO && stereo_mode == GLSU_SPLIT_HORIZONTALLY)
					y_offset = ctx.get_height() / 2;
			}
			// render all user views and take last eye into main render pass if no separate view is rendered
			int y0 = 0;
			for (unsigned i = 0; i < displays.size(); ++i) {
				auto kit_ptr = displays[i];
				int x0 = 0;
				int blit_height = (int)(blit_width * kit_ptr->get_height() / (blit_aspect_scale * kit_ptr->get_width()));
				for (int eye = 0; eye < 2; ++eye) {
					kit_ptr->blit_fbo(eye, x0 + x_offset, y0 + y_offset, blit_width, blit_height);
					x0 += blit_width + 5;
				}
				y0 += blit_height + 5;
			}
		}
		break;
		case MVM_TWO_PLAYERS_PASSIVE_ANAGLYPH:
		{
			if (stereo_enabled && displays.size() >= 2 && stereo_mode == GLSU_ANAGLYPH) {
				float tmp_gamma = ctx.get_gamma();
				ctx.set_gamma(1.0f);
				// in case of left eye
				if (ctx.get_render_pass() == cgv::render::RP_STEREO) {
					glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);
					displays[0]->bind_texture(0); cgv::render::gl::cover_screen(ctx, 0, false, -1.0f, -1.0f, 0.0f, 1.0f);
					displays[1]->bind_texture(0); cgv::render::gl::cover_screen(ctx, 0, false, 0.0f, -1.0f, 1.0f, 1.0f);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				}
				// in case of right eye
				else {
					glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);
					displays[0]->bind_texture(1); cgv::render::gl::cover_screen(ctx, 0, false, -1.0f, -1.0f, 0.0f, 1.0f);
					displays[1]->bind_texture(1); cgv::render::gl::cover_screen(ctx, 0, false, 0.0f, -1.0f, 1.0f, 1.0f);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				}
				ctx.set_gamma(tmp_gamma);
			}
		}
		break;
		case MVM_TWO_PLAYERS_PASSIVE_ACTIVE:
			if (stereo_enabled && displays.size() >= 2 && stereo_mode == GLSU_QUAD_BUFFER) {
				float tmp_gamma = ctx.get_gamma();
				ctx.set_gamma(1.0f);
				// in case of left eye
				if (ctx.get_render_pass() == cgv::render::RP_STEREO) {
					displays[0]->bind_texture(0); cgv::render::gl::cover_screen(ctx, 0, false, -1.0f, -1.0f, 0.0f, 1.0f);
					displays[1]->bind_texture(0); cgv::render::gl::cover_screen(ctx, 0, false, 0.0f, -1.0f, 1.0f, 1.0f);
				}
				// in case of right eye
				else {
					displays[0]->bind_texture(1); cgv::render::gl::cover_screen(ctx, 0, false, -1.0f, -1.0f, 0.0f, 1.0f);
					displays[1]->bind_texture(1); cgv::render::gl::cover_screen(ctx, 0, false, 0.0f, -1.0f, 1.0f, 1.0f);
				}
				ctx.set_gamma(tmp_gamma);
			}
			break;
		case MVM_FOUR_PLAYERS:
			if (stereo_enabled && displays.size() >= 4 && stereo_mode == GLSU_QUAD_BUFFER) {
				float tmp_gamma = ctx.get_gamma();
				ctx.set_gamma(1.0f);
				// in case of left eye
				if (ctx.get_render_pass() == cgv::render::RP_STEREO) {
					glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);
					displays[0]->bind_texture(0); cgv::render::gl::cover_screen(ctx, 0, false, -1.0f, -1.0f, 0.0f, 1.0f);
					displays[1]->bind_texture(0); cgv::render::gl::cover_screen(ctx, 0, false, 0.0f, -1.0f, 1.0f, 1.0f);
					glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);
					displays[2]->bind_texture(1); cgv::render::gl::cover_screen(ctx, 0, false, -1.0f, -1.0f, 0.0f, 1.0f);
					displays[3]->bind_texture(1); cgv::render::gl::cover_screen(ctx, 0, false, 0.0f, -1.0f, 1.0f, 1.0f);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				}
				// in case of right eye
				else {
					glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);
					displays[0]->bind_texture(0); cgv::render::gl::cover_screen(ctx, 0, false, -1.0f, -1.0f, 0.0f, 1.0f);
					displays[1]->bind_texture(0); cgv::render::gl::cover_screen(ctx, 0, false, 0.0f, -1.0f, 1.0f, 1.0f);
					glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);
					displays[2]->bind_texture(1); cgv::render::gl::cover_screen(ctx, 0, false, -1.0f, -1.0f, 0.0f, 1.0f);
					displays[3]->bind_texture(1); cgv::render::gl::cover_screen(ctx, 0, false, 0.0f, -1.0f, 1.0f, 1.0f);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				}
				ctx.set_gamma(tmp_gamma);
			}
			if (stereo_enabled && displays.size() >= 4 && stereo_mode == GLSU_SPLIT_HORIZONTALLY) {
				float tmp_gamma = ctx.get_gamma();
				ctx.set_gamma(1.0f);
				glDisable(GL_DEPTH_TEST);
				// in case of left eye
				if (ctx.get_render_pass() == cgv::render::RP_STEREO) {
					glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);
					displays[0]->bind_texture(0); cgv::render::gl::cover_screen(ctx, 0, false, -1.0f, -1.0f, 0.0f, 1.0f);
					displays[1]->bind_texture(0); cgv::render::gl::cover_screen(ctx, 0, false, 0.0f, -1.0f, 1.0f, 1.0f);
					glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);
					displays[0]->bind_texture(1); cgv::render::gl::cover_screen(ctx, 0, false, -1.0f, -1.0f, 0.0f, 1.0f);
					displays[1]->bind_texture(1); cgv::render::gl::cover_screen(ctx, 0, false, 0.0f, -1.0f, 1.0f, 1.0f);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				}
				// in case of right eye
				else {
					glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE);
					displays[2]->bind_texture(0); cgv::render::gl::cover_screen(ctx, 0, false, -1.0f, -1.0f, 0.0f, 0.0f);
					displays[3]->bind_texture(0); cgv::render::gl::cover_screen(ctx, 0, false, 0.0f, -1.0f, 1.0f, 0.0f);
					glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);
					displays[2]->bind_texture(1); cgv::render::gl::cover_screen(ctx, 0, false, -1.0f, -1.0f, 0.0f, 0.0f);
					displays[3]->bind_texture(1); cgv::render::gl::cover_screen(ctx, 0, false, 0.0f, -1.0f, 1.0f, 0.0f);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				}
				glEnable(GL_DEPTH_TEST);
				ctx.set_gamma(tmp_gamma);
			}
			break;
		}
	}
}

///
bool multi_view_interactor::handle(cgv::gui::event& e)
{
	if (vr_view_interactor::handle(e))
		return true;
	if (e.get_kind() != cgv::gui::EID_KEY)
		return false;
	auto& ke = reinterpret_cast<cgv::gui::key_event&>(e);
	if (ke.get_action() != cgv::gui::KA_RELEASE) {
		switch (ke.get_key()) {
		case cgv::gui::KEY_F9 :
			if (multi_view_mode == MVM_LAST)
				multi_view_mode = MVM_DEBUG;
			else
				++(int&)multi_view_mode;
			on_set(&multi_view_mode);
			return true;
		}
	}
	return false;
}

///
void multi_view_interactor::create_gui() 
{
	add_decorator("multi view interactor", "heading");
	add_member_control(this, "mutli-view mode", multi_view_mode, "dropdown", "enums='debug,passive+anaglyph,passive+active,four players'");
	if (begin_tree_node("screen definition", show_screen, false, "level=2")) {
		align("\a");
		add_member_control(this, "show_screen", show_screen, "check");
		add_member_control(this, "pixel_scale", pixel_scale, "value_slider", "min=0.00001;max=0.01;log=true;ticks=true;step=0.0000001");
		add_decorator("screen_orientation", "heading", "level=3");
		add_gui("screen_orientation", (vec4&)screen_orientation, "direction", "options='min=-1;max=1;ticks=true'");
		add_decorator("screen_center", "heading", "level=3");
		add_gui("screen_center", (vec3&)screen_pose(0, 3), "", "options='min=-2;max=2;ticks=true'");
		align("\b");
		end_tree_node(show_screen);
	}
	if (begin_tree_node("debug projection", debug_display_index, false, "level=2")) {
		align("\a");
		add_member_control(this, "show_eyes", show_eyes, "check");
		add_member_control(this, "debug_display_index", debug_display_index, "value_slider", "min=-1;max=-1;ticks=true");
		find_control(debug_display_index)->set("max", int(displays.size()) - 1);
		add_member_control(this, "debug_eye", (cgv::type::DummyEnum&)debug_eye, "dropdown", "enums='left,right'");
		add_member_control(this, "show_probe", show_probe, "check");
		add_decorator("debug_probe", "heading", "level=3");
		add_gui("debug_probe", debug_probe, "", "options='min=-2;max=2;ticks=true'");
		align("\b");
		end_tree_node(debug_display_index);
	}
	if (begin_tree_node("rectangle render style", rrs, false, "level=2")) {
		align("\a");
		add_gui("rectangle render style", rrs);
		align("\b");
		end_tree_node(rrs);
	}
	vr_view_interactor::create_gui();
}



#include <cgv/base/register.h>

/// register a newly created cube with the name "cube1" as constructor argument
extern cgv::base::object_registration_1<multi_view_interactor,const char*> 
 obj1("multi view interactor", "registration of multi view interactor");