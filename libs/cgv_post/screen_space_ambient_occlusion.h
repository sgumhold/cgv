#pragma once

#include <cgv_post/post_process_effect.h>

#include "lib_begin.h"

namespace cgv {
namespace post {

class CGV_API screen_space_ambient_occlusion : public post_process_effect {
protected:
	/// framebuffers used to store raw and blurred occlusion term
	cgv::render::managed_frame_buffer fbc_post, fbc_blur;
	/// the sample positions inside the hemisphere
	std::vector<vec3> sample_offsets;
	/// provides random directions for orienting the sample hemisphere per pixel
	cgv::render::texture noise_tex;
	/// strength scale of the occlusion term
	float strength = 1.0f;
	/// radius of the sample hemisphere
	float radius = 0.5f;
	/// depth difference bias to prevent false occlusion
	float bias = 0.025f;
	/// generate random samples and noise texture
	void generate_samples_and_noise_texture(cgv::render::context& ctx);

	void create_gui_impl(cgv::base::base* b, cgv::gui::provider* p);

public:
	screen_space_ambient_occlusion() : post_process_effect("SSAO") {}

	void destruct(cgv::render::context& ctx);

	bool init(cgv::render::context& ctx);

	bool ensure(cgv::render::context& ctx);

	void begin(cgv::render::context& ctx);

	void end(cgv::render::context& ctx);

	float get_strength() const { return strength; }

	void set_strength(float s) { set_and_update_member(strength, s); }

	float get_radius() const { return radius; }

	void set_radius(float r) { set_and_update_member(radius, r); }

	float get_bias() const { return bias; }

	void set_bias(float b) { set_and_update_member(bias, b); }
};

}
}

#include <cgv/config/lib_end.h>
