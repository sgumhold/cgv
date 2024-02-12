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
	bool has_damage_ = true;
	bool recreate_layout_requested_ = true;
	bool blending_was_enabled_ = false;

	cgv::g2d::shape2d_style blit_style_;

	cgv::render::managed_frame_buffer frame_buffer_;

protected:
	cgv::g2d::canvas overlay_canvas, content_canvas;

	/// whether to enable blending during the draw process
	bool blend_overlay = false;
	
	bool ensure_layout(cgv::render::context& ctx);

	void post_recreate_layout();

	void clear_damage();

	bool is_damaged() const;

	void begin_content(cgv::render::context& ctx, bool clear_frame_buffer = true);
	
	void end_content(cgv::render::context& ctx, bool keep_damage = false);

	void enable_blending();

	void disable_blending();

	void draw_impl(cgv::render::context& ctx);

	virtual void init_styles() {}

public:
	/// creates an overlay in the bottom left corner with zero size using a canvas for 2d drawing
	canvas_overlay();

	bool init(cgv::render::context& ctx) override;

	void clear(cgv::render::context& ctx) override;

	/// default implementation of that calls handle_member_change and afterwards upates the member in the gui and post damage to the canvas overlay
	virtual void on_set(void* member_ptr) override;
	/// implement to handle member changes
	virtual void handle_member_change(const cgv::utils::pointer_test& m) override {}

	void draw(cgv::render::context& ctx) override;
	void finish_frame(cgv::render::context&) override;
	virtual void draw_content(cgv::render::context& ctx) = 0;

	void register_shader(const std::string& name, const std::string& filename);

	virtual void handle_theme_change(const cgv::gui::theme_info& theme) override;

	void post_damage(bool redraw = true);
};

typedef cgv::data::ref_ptr<canvas_overlay> canvas_overlay_ptr;

}
}

#include <cgv/config/lib_end.h>
