#pragma once

#include <cgv_post/post_process_effect.h>

#include "lib_begin.h"

namespace cgv {
namespace post {

class CGV_API depth_halos : public post_process_effect {
protected:
	/// strength scale of the halo darkening
	float strength = 1.0f;
	/// halo radius in pixel
	float radius = 10.0f;
	/// depth difference threshold
	float threshold = 0.5f;
	/// used to scale the depth value to adjust the effect to different scene extents
	float depth_scale = 1.0f;
	/// provides random offsets for depth samples
	cgv::render::texture noise_tex;
	/// generate random samples and noise texture
	void generate_noise_texture(cgv::render::context& ctx);

public:
	depth_halos() : post_process_effect("Depth Halos") {}

	void destruct(cgv::render::context& ctx);

	bool init(cgv::render::context& ctx);

	bool ensure(cgv::render::context& ctx);

	void begin(cgv::render::context& ctx);

	void end(cgv::render::context& ctx);

	void create_gui(cgv::gui::provider* p);

	float get_strength() const { return strength; }

	void set_strength(float s) { strength = s; }

	float get_radius() const { return radius; }

	void set_radius(float r) { radius = r; }

	float get_threshold() const { return threshold; }

	void set_threshold(float t) { threshold = t; }

	float get_depth_scale() const { return depth_scale; }

	void set_depth_scale(float s) { depth_scale = s; }
};

}
}

#include <cgv/config/lib_end.h>
