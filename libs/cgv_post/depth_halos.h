#pragma once

#include <cgv_post/post_process_effect.h>

#include "lib_begin.h"

namespace cgv {
namespace post {

class CGV_API depth_halos : public post_process_effect {
public:
	enum class Mode {
		Inside,
		Outside,
		Center
	};

protected:
	/// provides random offsets for depth samples
	cgv::render::texture noise_tex;
	/// whether to reload the depth halo shader
	bool do_reload_shader = false;
	/// halo mode
	Mode mode = Mode::Outside;
	/// strength scale of the halo darkening
	float strength = 1.0f;
	/// halo radius in pixel
	float radius = 10.0f;
	/// depth difference threshold
	float threshold = 0.5f;
	/// used to scale the depth value to adjust the effect to different scene extents
	float depth_scale = 1.0f;
	/// return shader defines dependent on current settings
	cgv::render::shader_define_map get_shader_defines();
	/// mode change callback handler
	void on_change_mode();
	/// generate random samples and noise texture
	void generate_noise_texture(cgv::render::context& ctx);

	void create_gui_impl(cgv::base::base* b, cgv::gui::provider* p);

public:
	depth_halos() : post_process_effect("Depth Halos") {}

	void destruct(cgv::render::context& ctx);

	bool init(cgv::render::context& ctx);

	bool ensure(cgv::render::context& ctx);

	void begin(cgv::render::context& ctx);

	void end(cgv::render::context& ctx);

	/// accessors
	Mode get_mode() const { return mode; }

	void set_mode(Mode m) { set_and_update_member(mode, m); on_change_mode(); }
	
	float get_strength() const { return strength; }

	void set_strength(float s) { set_and_update_member(strength, s); }

	float get_radius() const { return radius; }

	void set_radius(float r) { set_and_update_member(radius, r); }

	float get_threshold() const { return threshold; }

	void set_threshold(float t) { set_and_update_member(threshold, t); }

	float get_depth_scale() const { return depth_scale; }

	void set_depth_scale(float s) { set_and_update_member(depth_scale, s); }
};

}
}

#include <cgv/config/lib_end.h>
