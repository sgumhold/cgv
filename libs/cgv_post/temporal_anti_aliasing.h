#pragma once

#include <cgv_post/post_process_effect.h>

#include "lib_begin.h"

namespace cgv {
namespace post {

class CGV_API temporal_anti_aliasing : public post_process_effect {
protected:
	/// framebuffers used to store previous and resovled frame
	cgv::render::managed_frame_buffer fbc_post, fbc_hist, fbc_resolve;
	/// pointer to the current view
	cgv::render::view* view_ptr = nullptr;
	/// struct to hold necessary view information
	struct view_parameters {
		/// the eye position (used to detect static frames)
		vec3 eye_pos = vec3(0.0f);
		/// the view direction (used to detect static frames)
		vec3 view_dir = vec3(0.0f);
		/// the view up direction (used to detect static frames)
		vec3 view_up_dir = vec3(0.0f);
		/// the view-projection matrix used to reproject the current onto the previous position
		mat4 view_projection_matrix;
	};
	/// store view parameters of the current frame
	view_parameters current_view;
	/// store view parameters of the previous frame
	view_parameters previous_view;
	/// the viewport dependant jitter sample offsets
	std::vector<vec2> jitter_offsets;
	/// number of so-far accumulated frames (is reset upon reaching <jitter_sample_count>)
	unsigned accumulate_count = 0;
	/// keep track of the number of static frames rendered (no camera movement),
	/// to request new frames if the accumulation is not finished
	unsigned static_frame_count = 0;
	/// if temporal accumulation is active
	bool accumulate = false;
	/// influence of the new frame on the history color
	float mix_factor = 0.1f;
	/// the number of samples used to jitter the projection matrix
	size_t jitter_sample_count = 16;
	/// scale for the jitter offsets
	float jitter_scale = 0.5f;
	/// whether to use the camera movement velocity to offset history buffer samples (reduces motion blur)
	bool use_velocity = true;
	/// whether to enable fast approximate anti-aliasing before accumulation (FXAA also works with disabled TAA)
	bool enable_fxaa = true;
	/// influence factor of fxaa result with input image
	float fxaa_mix_factor = 0.75f;
	/// implements a van der corput sequence to generate 1d low discrepancy sequences of numbers in the unit interval
	float van_der_corput(int n, int base) const;
	/// generate 2d samples from a halton sequence using the underlying van der corput sequences
	vec2 sample_halton_2d(unsigned k, int base1, int base2) const;
	/// generate <jitter_sample_count> number of pseudo-random 2d samples
	void generate_jitter_offsets();
	/// returns true if the view appears to be unchanged since the last frame (tests eye position, view direction and view up direction)
	bool is_static_view() const;

	void create_gui_impl(cgv::base::base* b, cgv::gui::provider* p);

public:
	temporal_anti_aliasing() : post_process_effect("TAA") {}

	void destruct(cgv::render::context& ctx);

	bool init(cgv::render::context& ctx);

	bool ensure(cgv::render::context& ctx);

	void reset();

	void reset_static_frame_count();

	void begin(cgv::render::context& ctx);

	void end(cgv::render::context& ctx);

	void set_view(cgv::render::view* view) { view_ptr = view; }

	bool is_fxaa_enabled() const { return enable_fxaa; }

	bool is_velocity_enabled() const { return use_velocity; }

	float get_mix_factor() const { return mix_factor; }

	float get_fxaa_mix_factor() const { return fxaa_mix_factor; }

	float get_jitter_scale() const { return jitter_scale; }

	size_t get_jitter_sample_count() const { return jitter_sample_count; }
	/// return the 2d jitter offset according to the current accumulation count (warning: viewport dependent)
	vec2 get_current_jitter_offset() const;
	/// return the normalized 2d jitter offset according to the current accumulation count
	vec2 get_normalized_current_jitter_offset() const { return get_current_jitter_offset() * viewport_size; }
	/// return the current accumulation count
	unsigned int get_current_accumulation_count() const { return accumulate_count; }

	void set_fxaa_enabled(bool enable) { set_and_update_member(enable_fxaa, enable); }

	void set_velocity_enabled(bool enable) { set_and_update_member(use_velocity, enable); }

	void set_mix_factor(float value) { set_and_update_member(mix_factor, std::min(std::max(value, 0.0f), 1.0f)); }

	void set_fxaa_mix_factor(float value) { set_and_update_member(fxaa_mix_factor, std::min(std::max(value, 0.0f), 1.0f)); }

	void set_jitter_scale(float value) { set_and_update_member(jitter_scale, std::min(std::max(value, 0.0f), 2.0f)); }

	void set_jitter_sample_count(size_t count) { set_and_update_member(jitter_sample_count, std::min(std::max(count, size_t(1)), size_t(128))); }

	const std::vector<vec2> ref_jitter_offsets() const {
		return jitter_offsets;
	}

	const vec2 ref_viewport_size() const {
		return viewport_size;
	}
};

}
}

#include <cgv/config/lib_end.h>
