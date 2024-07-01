#pragma once

#include <cgv/render/clipped_view.h>
#include <cgv_post/post_process_effect.h>

#include "lib_begin.h"

namespace cgv {
namespace post {

class CGV_API depth_masking : public post_process_effect {
public:
	enum class Mode {
		kInside,
		kOutside,
		kCenter,
		kInAndOutside
	};

protected:
	/// framebuffers used to store depth information (linear depth and mask) and blurred depth values
	cgv::render::managed_frame_buffer fbc_depth_info, fbc_blurred_depth_ping, fbc_blurred_depth_pong;
	/// pointer to the current view of type clipped view
	cgv::render::clipped_view* view = nullptr;
	/// masking mode
	Mode mode = Mode::kOutside;
	/// strength scale of the masking
	float strength = 1.0f;
	/// mask blur radius as percentage of the screen diagonal in pixels
	float radius_percentage = 0.02f;
	/// numer of box blur iterations used to approximate gaussian blur
	unsigned blur_iterations = 3;
	/// whether to tint the masked areas using the inner and outer colors
	bool tint_colors = false;
	/// color used in inside (foreground) masked areas
	cgv::rgb inside_color = { 1.0f, 1.0f, 0.0f };
	/// color used in outside (background) masked areas
	cgv::rgb outside_color = { 0.0f, 0.0f, 1.0f };
	/// whether to clamp the output color to [0,1]
	bool clamp_output = true;

	void create_gui_impl(cgv::base::base* b, cgv::gui::provider* p);

public:
	depth_masking() : post_process_effect("Depth Masking") {}

	void destruct(cgv::render::context& ctx);

	bool init(cgv::render::context& ctx);

	bool ensure(cgv::render::context& ctx);

	void begin(cgv::render::context& ctx);

	void end(cgv::render::context& ctx);

	/// accessors
	void set_view(cgv::render::view* view) { this->view = dynamic_cast<cgv::render::clipped_view*>(view); }

	Mode get_mode() const { return mode; }

	void set_mode(Mode m) { set_and_update_member(mode, m); }
	
	float get_strength() const { return strength; }

	void set_strength(float s) { set_and_update_member(strength, s); }
	
	float get_radius_percentage() const { return radius_percentage; }

	void set_radius_percentage(float r) { set_and_update_member(radius_percentage, r); }

	unsigned get_blur_iterations() const { return blur_iterations; }

	void set_blur_iterations(unsigned n) { set_and_update_member(blur_iterations, n); }

	bool get_tint_colors() const { return tint_colors; }

	void set_tint_colors(bool f) { set_and_update_member(tint_colors, f); }

	rgb get_inside_color() const { return inside_color; }

	void set_inside_color(rgb c) { set_and_update_member(inside_color, c); }

	rgb get_outside_color() const { return outside_color; }

	void set_outside_color(rgb c) { set_and_update_member(outside_color, c); }
};

}
}

#include <cgv/config/lib_end.h>
