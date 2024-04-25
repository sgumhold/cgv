#include "vr_wall_kit.h"
#include <cgv/math/ftransform.h>
#include <cgv_gl/gl/gl.h>
#include <libs/vr/vr_driver.h>
///
namespace vr {
	void vr_wall_kit::detach()
	{
		if (!is_attached())
			return;
		replace_by_pointer(this, parent_kit);
		parent_kit = 0;
	}
	/// change the parent
	bool vr_wall_kit::attach(int vr_kit_parent_index)
	{
		if (vr_kit_parent_index < 0) {
			detach();
			return true;
		}
		if (is_attached())
			detach();
		parent_kit = replace_by_index(vr_kit_parent_index, this);
		camera = parent_kit->get_camera();
		driver = const_cast<vr::vr_driver*>(parent_kit->get_driver());
		handle = parent_kit->get_handle();
		info = parent_kit->get_device_info();
		std::cout << "attached kit:\n" << info << std::endl;
		return true;
	}

	/// initialize render targets and framebuffer objects in current opengl context
	bool vr_wall_kit::init_fbos(EyeSelection es)
	{
		if (wall_context) {
			if (!gl_vr_display::fbos_initialized(es)) {
				// check whether we can do share render buffers and textures
				gl_vr_display* parent_gl_kit = dynamic_cast<gl_vr_display*>(parent_kit);
				if (!parent_gl_kit || !parent_gl_kit->fbos_initialized(es))
					return false;
					//return gl_vr_display::init_fbos();
				// create fbos with textures shared with parent kit
				
				//std::cout << "init_fbos(wall): " << wglGetCurrentContext() << std::endl;
				
				for (unsigned i = 0; i < 2; ++i) {
					if (es == ES_LEFT && i == 1)
						continue;
					if (es == ES_RIGHT && i == 0)
						continue;

					glGenFramebuffers(1, &multi_fbo_id[i]);
					glBindFramebuffer(GL_FRAMEBUFFER, multi_fbo_id[i]);
					multi_depth_buffer_id[i] = parent_gl_kit->multi_depth_buffer_id[i];
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, multi_depth_buffer_id[i]);
					multi_tex_id[i] = parent_gl_kit->multi_tex_id[i];
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, multi_tex_id[i], 0);
					if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
						destruct_fbos();
						glBindFramebuffer(GL_FRAMEBUFFER, 0);
						last_error = "check status of multi framebuffer failed";
						return false;
					}
					glGenFramebuffers(1, &fbo_id[i]);
					glBindFramebuffer(GL_FRAMEBUFFER, fbo_id[i]);
					tex_id[i] = parent_gl_kit->tex_id[i];
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id[i], 0);
					if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
						destruct_fbos();
						glBindFramebuffer(GL_FRAMEBUFFER, 0);
						last_error = "check status of framebuffer failed";
						return false;
					}
				}
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				return true;
			}
		}
		else {
			// std::cout << "init_fbos(main): " << wglGetCurrentContext() << std::endl;
			if (!parent_kit->fbos_initialized(es))
				return parent_kit->init_fbos(es);
		}
		return false;
	}
	/// check whether fbos have been initialized
	bool vr_wall_kit::fbos_initialized(EyeSelection es) const
	{
		if (wall_context)
			return gl_vr_display::fbos_initialized(es);
		else {
			// ensure same size of textures
			if (parent_kit->get_width() != width || parent_kit->get_height() != height) {
				if (parent_kit->fbos_initialized(es))
					parent_kit->destruct_fbos(es);
				dynamic_cast<vr::gl_vr_display*>(parent_kit)->set_size(width, height);
			}
			return parent_kit->fbos_initialized(es);
		}
	}
	/// destruct render targets and framebuffer objects in current opengl context
	void vr_wall_kit::destruct_fbos(EyeSelection es)
	{
		if (wall_context)
			gl_vr_display::destruct_fbos(es);
		else
			parent_kit->destruct_fbos(es);
	}
	void vr_wall_kit::ensure_fbo(int eye)
	{
		EyeSelection es = eye == 0 ? ES_LEFT : ES_RIGHT;
		if (fbos_initialized(es))
			return;
		init_fbos(es);
	}
	/// enable the framebuffer object of given eye (0..left, 1..right) 
	void vr_wall_kit::enable_fbo(int eye)
	{
		if (wall_context) {
			ensure_fbo(eye);
			gl_vr_display::enable_fbo(eye);
		}
		else
			parent_kit->enable_fbo(eye);
	}
	/// disable the framebuffer object of given eye
	void vr_wall_kit::disable_fbo(int eye)
	{
		if (wall_context)
			gl_vr_display::disable_fbo(eye);
		else
			parent_kit->disable_fbo(eye);
	}
	/// initialize render targets and framebuffer objects in current opengl context
	bool vr_wall_kit::blit_fbo(int eye, int x, int y, int w, int h)
	{
		if (wall_context) {
			ensure_fbo(eye);
			return gl_vr_display::blit_fbo(eye, x, y, w, h);
		}
		return parent_kit->blit_fbo(eye, x, y, w, h);
	}
	/// bind texture of given eye to current texture unit
	void vr_wall_kit::bind_texture(int eye)
	{
		if (wall_context)
			return gl_vr_display::bind_texture(eye);
		else
			return parent_kit->bind_texture(eye);
	}
	/// transform to coordinate system of screen with [0,0,0] in center and corners [+-aspect,+-1,0]; z is signed distance to screen in world unites (typically meters) 
	vr_wall_kit::vec3 vr_wall_kit::transform_world_to_screen(const vec3& p) const
	{
		vec3 p_screen = (p - get_screen_center()) * get_screen_orientation();
		p_screen[0] /= 0.5f * height * pixel_size[0];
		p_screen[1] /= 0.5f * height * pixel_size[1];
		return p_screen;
	}
	/// transform from coordinate system of screen with [0,0,0] in center and corners [+-aspect,+-1,0]; z is signed distance to screen in world unites (typically meters) 
	vr_wall_kit::vec3 vr_wall_kit::transform_screen_to_world(const vec3& p_screen) const
	{
		vec3 p_s = p_screen;
		p_s[0] *= 0.5f * height * pixel_size[0];
		p_s[1] *= 0.5f * height * pixel_size[1];
		return get_screen_orientation()* p_s + get_screen_center();
	}
	/// construct vr wall kit by attaching to another vr kit
	vr_wall_kit::vr_wall_kit(int vr_kit_parent_index, unsigned _width, unsigned _height, const std::string& _name) :
		vr_kit(0, 0, _name, width, height)
	{
		wall_context = false;
		skip_calibration = true;
		in_calibration = false;
		parent_kit = 0;
		if (attach(vr_kit_parent_index))
			camera = parent_kit->get_camera();
		else
			camera = 0;
		screen_pose.identity();
		pixel_size = vec2(0.001f);
		width = _width;
		height = _height;
		eye_center_tracker.zeros();
		eye_separation_dir_tracker = vec3(1.0f, 0, 0);
		eye_separation = 0.07f;
		hmd_trackable_index = -1;
	}

	bool vr_wall_kit::set_vibration(unsigned controller_index, float low_frequency_strength, float high_frequency_strength)
	{
		return parent_kit->set_vibration(controller_index, low_frequency_strength, high_frequency_strength);
	}
	/// update state by swapping information of hmd and trackable used to emulate hmd
	bool vr_wall_kit::query_state_impl(vr_kit_state& state, int pose_query) 
	{
		bool res = parent_kit->query_state(state, pose_query);
		if (res && !in_calibration) {
			if (hmd_trackable_index != -1)
				std::swap(state.hmd, static_cast<vr_trackable_state&>(state.controller[hmd_trackable_index]));
		}
		return res;
	}
	/// access to 3x4 matrix in column major format for transformation from eye (0..left, 1..right) to head coordinates
	void vr_wall_kit::put_eye_to_head_matrix(int eye, float* pose_matrix) const 
	{
		if (in_calibration)
			parent_kit->put_eye_to_head_matrix(eye, pose_matrix);
		else {
			mat34& pose = *reinterpret_cast<mat34*>(pose_matrix);
			mat3& orientation = *reinterpret_cast<mat3*>(pose_matrix);
			orientation.identity();
			vec3& position = *reinterpret_cast<vec3*>(pose_matrix + 9);
			position = eye_center_tracker + eye_separation * (eye - 0.5f)*eye_separation_dir_tracker;
		}
	}
	/// access to 4x4 matrix in column major format for perspective transformation from eye (0..left, 1..right)
	void vr_wall_kit::put_projection_matrix(int eye, float z_near, float z_far, float* projection_matrix, const float* hmd_pose) const
	{
		if (in_calibration)
			parent_kit->put_projection_matrix(eye, z_near, z_far, projection_matrix);
		else {
			if (hmd_pose == 0) {
				std::cerr << "ERROR in vr_wall_kit::put_projection_matrix() ... hmd pose not provided!" << std::endl;
			}
			else {
				vec3 eye_world = get_eye_position_world(eye, *reinterpret_cast<const mat34*>(hmd_pose));
				vec3 eye_screen = (eye_world - get_screen_center()) * get_screen_orientation();
				float scale = z_near / eye_screen(2);
				float width_world = get_width()*pixel_size[0];
				float l = scale * (-0.5f*width_world - eye_screen(0));
				float r = scale * (+0.5f*width_world - eye_screen(0));
				float height_world = get_height()*pixel_size[1];
				float b = scale * (-0.5f*height_world - eye_screen(1));
				float t = scale * (+0.5f*height_world - eye_screen(1));
				reinterpret_cast<cgv::mat4&>(*projection_matrix) = cgv::math::frustum4<float>(l, r, b, t, z_near, z_far);
			}
		}
	}
	/// compute lookat matrix for a given eye (0 ... left, 1 ... right)
	void vr_wall_kit::put_world_to_eye_transform(int eye, const float* hmd_pose, float* modelview_matrix) const
	{
		if (in_calibration)
			parent_kit->put_world_to_eye_transform(eye, hmd_pose, modelview_matrix);
		else {
			vec3 eye_world = get_eye_position_world(eye, *reinterpret_cast<const mat34*>(hmd_pose));
			mat3 R = get_screen_orientation();
			R.transpose();
			cgv::mat4& T = reinterpret_cast<cgv::mat4&>(*modelview_matrix);
			T.set_col(0, vec4(R.col(0), 0));
			T.set_col(1, vec4(R.col(1), 0));
			T.set_col(2, vec4(R.col(2), 0));
			T.set_col(3, vec4(-R * eye_world, 1));
		}
	}
	/// submit the rendered stereo frame to the hmd
	void vr_wall_kit::submit_frame()
	{
		on_submit_frame(this);
	}
}
