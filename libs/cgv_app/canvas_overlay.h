#pragma once

#include <cgv/render/managed_frame_buffer.h>
#include <cgv_app/overlay.h>
#include <cgv_g2d/canvas.h>
#include <cgv_g2d/shape2d_styles.h>
#include <cgv/gui/theme_info.h>

#include "lib_begin.h"

namespace cgv {
namespace app {

class CGV_API canvas_overlay : public overlay, public cgv::gui::theme_observer {
private:
	bool has_damage = true;
	bool recreate_layout = true;
	GLboolean blending_was_enabled = false;

protected:
	cgv::render::managed_frame_buffer fbc;

	cgv::g2d::canvas content_canvas, overlay_canvas;

	bool blend_overlay = false;
	
	void init_overlay_style(cgv::render::context& ctx);

	bool ensure_layout(cgv::render::context& ctx);

	void post_recreate_layout();

	void clear_damage();

	bool is_damaged() const;

	void begin_content(cgv::render::context& ctx, bool clear_frame_buffer = true);
	
	void end_content(cgv::render::context& ctx, bool keep_damage = false);

	void enable_blending();

	void disable_blending();

	virtual void init_styles(cgv::render::context& ctx) {}

public:
	/// creates an overlay in the bottom left corner with zero size using a canvas for 2d drawing
	canvas_overlay();

	void clear(cgv::render::context& ctx);

	bool init(cgv::render::context& ctx);
	void draw(cgv::render::context& ctx);
	virtual void draw_content(cgv::render::context& ctx) = 0;

	void register_shader(const std::string& name, const std::string& filename);

	virtual void handle_theme_change(const cgv::gui::theme_info& theme) override;

	void post_damage(bool redraw = true);
};

typedef cgv::data::ref_ptr<canvas_overlay> canvas_overlay_ptr;

}
}

#include <cgv/config/lib_end.h>
