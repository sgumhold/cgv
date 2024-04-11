#pragma once

#include <vr/vr_kit.h>
#include <vr/vr_state.h>
#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/math/quaternion.h>
#include <cgv/signal/signal.h>

#include "lib_begin.h"

///@ingroup VR
///@{

///
namespace vr {
	class CGV_API vr_wall_kit : public vr_kit
	{
	public:
		using vec2 = cgv::vec2;
		using vec3 = cgv::vec3;
		using vec4 = cgv::vec4;
		using mat3 = cgv::mat3;
		using mat34 = cgv::mat34;
		using quat = cgv::quat;

	protected:
		/// vr kit to which to be attached
		vr_kit* parent_kit;
		/// allow vr_wall to access the protected members in order to create gui and incorporate them into reflection
		friend class vr_wall;
		/// store screen coordinate and position in a way that it can be cast into a pose matrix
		mat34 screen_pose;
		/// x- and y-pixel sizes in meters
		vec2 pixel_size;
		/// head calibration information
		vec3 eye_center_tracker;
		vec3 eye_separation_dir_tracker;
		float eye_separation;
		/// index of trackable used for head tracking (-1 if parent hmd is used)
		int hmd_trackable_index;

		bool wall_context;

		bool in_calibration;
		/// update state by swapping information of hmd and trackable used to emulate hmd
		bool query_state_impl(vr_kit_state& state, int pose_query);
	public:
		inline const mat3& get_screen_orientation() const { return reinterpret_cast<const mat3&>(screen_pose); }
		inline const vec3& get_screen_center() const { return screen_pose.col(3); }
		inline void set_screen_orientation(const quat& q) { q.put_matrix(reinterpret_cast<mat3&>(screen_pose)); }
		/// transform to coordinate system of screen with [0,0,0] in center and corners [+-aspect,+-1,0]; z is signed distance to screen in world unites (typically meters) 
		vec3 transform_world_to_screen(const vec3& p) const;
		/// transform from coordinate system of screen with [0,0,0] in center and corners [+-aspect,+-1,0]; z is signed distance to screen in world unites (typically meters) 
		vec3 transform_screen_to_world(const vec3& p) const;
		/// eye = 0 ..left | 1 ..right
		inline vec3 get_eye_position_tracker(int eye) const { return eye_center_tracker + ((eye-0.5f)*eye_separation)*eye_separation_dir_tracker; }
		/// eye = 0 ..left | 1 ..right
		inline vec3 get_eye_position_world(int eye, const mat34& tracker_pose) const { return tracker_pose * vec4(get_eye_position_tracker(eye), 1.0f); }
		/// construct vr wall kit by attaching to another vr kit
		vr_wall_kit(int vr_kit_parent_index, unsigned width, unsigned height, const std::string& _name);
		/// return pointer to parent to which wall kit is attached or nullptr if not attached
		inline vr_kit* get_parent() const { return parent_kit; }
		/// attach wall vr kit to other parent (if already attached, it is first detached)
		bool attach(int vr_kit_parent_index);
		/// return whether this wall kit is attached to a parent kit
		bool is_attached() const { return parent_kit != 0; }
		/// detach wall vr kit from current parent
		void detach();
		/// initialize render targets and framebuffer objects in current opengl context
		bool init_fbos(EyeSelection es = ES_BOTH);
		/// ensure that fbo of given eye is initialized
		void ensure_fbo(int eye);
		/// enable the framebuffer object of given eye (0..left, 1..right) 
		void enable_fbo(int eye);
		/// disable the framebuffer object of given eye
		void disable_fbo(int eye);
		/// initialize render targets and framebuffer objects in current opengl context
		bool blit_fbo(int eye, int x, int y, int w, int h);
		/// bind texture of given eye to current texture unit
		void bind_texture(int eye);

		/// check whether fbos have been initialized
		bool fbos_initialized(EyeSelection es = ES_BOTH) const;
		/// destruct render targets and framebuffer objects in current opengl context
		void destruct_fbos(EyeSelection es = ES_BOTH);
		///
		bool set_vibration(unsigned controller_index, float low_frequency_strength, float high_frequency_strength);
		/// access to 3x4 matrix in column major format for transformation from eye (0..left, 1..right) to head coordinates
		void put_eye_to_head_matrix(int eye, float* pose_matrix) const;
		/// access to 4x4 matrix in column major format for perspective transformation from eye (0..left, 1..right)
		void put_projection_matrix(int eye, float z_near, float z_far, float* projection_matrix, const float* hmd_pose) const;
		/// compute lookat matrix for a given eye (0 ... left, 1 ... right)
		void put_world_to_eye_transform(int eye, const float* hmd_pose, float* modelview_matrix) const;
		/// calls the on_submit_frame signal
		void submit_frame();
		/// emitted if new frame is submitted
		cgv::signal::signal<vr_wall_kit*> on_submit_frame;
	};
}

#include <cgv/config/lib_end.h>
