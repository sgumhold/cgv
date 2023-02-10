#pragma once

#include <cgv/defines/assert.h>
#include <cgv/gui/provider.h>
#include <cgv/render/context.h>
#include <cgv/render/managed_frame_buffer.h>
#include <cgv/render/render_types.h>
#include <cgv/render/shader_library.h>
#include <cgv/render/view.h>
#include <cgv_gl/gl/gl_context.h>

#include "lib_begin.h"

namespace cgv {
namespace post {

class CGV_API temporal_anti_aliasing : public cgv::render::render_types {
protected:
	/// framebuffers used to store images
	cgv::render::managed_frame_buffer fbc_draw, fbc_post, fbc_hist, fbc_resolve;
	/// shader library to manage used shaders
	cgv::render::shader_library shaders;

	/// the size of the current viewport
	vec2 viewport_size = vec2(0);

	/// whether to automatically call post_redraw()
	bool auto_redraw = true;

	/// the number of samples used to jitter the projection matrix
	size_t jitter_sample_count = 16;

	/// the viewport dependant jitter sample offsets
	std::vector<vec2> jitter_offsets;

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

	/// whether the class has been initialized
	bool is_initialized = false;

	/// if temporal accumulation is active
	bool accumulate = false;

	/// number of so-far accumulated frames (is reset upon reaching <jitter_sample_count>)
	unsigned accumulate_count = 0;

	/// keep track of the number of static frames (no camera movement) rendered,
	/// to request new frames if the accumulation is not finished
	unsigned static_frame_count = 0;

	/// TAA settings:
	/// whether to enable temporal anti-aliasing
	bool enable_taa = true;

	/// influence of the new frame on the history color
	float mix_factor = 0.1f;

	/// scale for the jitter offsets
	float jitter_scale = 0.5f;

	/// whether to use an offset calculated from the velocity of the camera movement for finding previous pixel values
	bool use_velocity = true;

	/// FXAA settings:
	/// whether to enable fast approximate anti-aliasing before accumulation
	bool enable_fxaa = true;

	/// influence factor of fxaa result with input image
	float fxaa_mix_factor = 0.75f;

	/// implements a van der corput sequence to generate 1d low discrepancy sequences of numbers in the unit interval
	float van_der_corput(int n, int base) const;

	/// generate 2d samples from a halton sequence using the underlying van der corput sequences
	vec2 sample_halton_2d(unsigned k, int base1, int base2) const;

	/// generate <jitter_sample_count> number of pseudo-random 2d samples
	void generate_jitter_offsets();

	/// return the 2d jitter offset according to the current accumulation count
	vec2 get_current_jitter_offset() const;

	/// returns true if the view appears to be unchanged since the last frame (tests eye position, view direction and view up direction)
	bool is_static_view() const;

public:
	temporal_anti_aliasing();

	void clear(cgv::render::context& ctx);

	bool init(cgv::render::context& ctx);

	bool ensure(cgv::render::context& ctx, bool force_update = false);

	bool is_fxaa_enabled() const { return enable_fxaa; }

	bool is_taa_enabled() const { return enable_taa; }

	bool is_velocity_enabled() const { return use_velocity; }

	float get_mix_factor() const { return mix_factor; }

	float get_fxaa_mix_factor() const { return fxaa_mix_factor; }

	float get_jitter_scale() const { return jitter_scale; }

	void set_fxaa_enabled(bool enable) { enable_fxaa = enable; }

	void set_taa_enabled(bool enable) { enable_taa = enable; }

	void set_velocity_enabled(bool enable) { use_velocity = enable; }

	void set_mix_factor(float value) { mix_factor = std::min(std::max(value, 0.0f), 1.0f); }
	
	void set_fxaa_mix_factor(float value) { fxaa_mix_factor = std::min(std::max(value, 0.0f), 1.0f); }

	void set_jitter_scale(float value) { fxaa_mix_factor = std::min(std::max(value, 0.0f), 2.0f); }

	const std::vector<vec2> ref_jitter_offsets() const {
		return jitter_offsets;
	}

	const vec2 ref_viewport_size() const {
		return viewport_size;
	}

	void reset();

	void begin(cgv::render::context& ctx, cgv::render::view* view_ptr);

	bool end(cgv::render::context& ctx);

	void create_gui(cgv::gui::provider* p);
};

}
}

#include <cgv/config/lib_end.h>
