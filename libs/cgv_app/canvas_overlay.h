#pragma once

#include <cgv/render/managed_frame_buffer.h>
#include <cgv_app/overlay.h>
#include <cgv_g2d/canvas.h>
#include <cgv_g2d/shape2d_styles.h>
#include <cgv/gui/theme_info.h>

#include "lib_begin.h"

namespace cgv {
namespace app {

class CGV_API canvas_overlay : public overlay {
private:
	bool has_damage_ = true;
	bool recreate_layout_requested_ = true;

	cgv::g2d::shape2d_style blit_style_;

	cgv::render::managed_frame_buffer frame_buffer_;

protected:
	cgv::g2d::canvas overlay_canvas, content_canvas;

	bool ensure_layout(cgv::render::context& ctx);

	void post_recreate_layout();

	void clear_damage();

	bool is_damaged() const;

	void begin_content(cgv::render::context& ctx, bool clear_frame_buffer = true);
	
	void end_content(cgv::render::context& ctx, bool keep_damage = false);

	virtual void draw_impl(cgv::render::context& ctx);

	virtual void init_styles() {}

	void register_shader(const std::string& name, const std::string& filename);

public:
	/// creates an overlay in the bottom left corner with zero size using a canvas for 2d drawing
	canvas_overlay();

	bool init(cgv::render::context& ctx) override;

	void clear(cgv::render::context& ctx) override;

	/// default implementation of that calls handle_member_change and afterwards upates the member in the gui and post damage to the canvas overlay
	virtual void on_set(void* member_ptr) override;
	/// implement to handle member changes
	virtual void handle_member_change(const cgv::utils::pointer_test& m) override {}
	/// draw the content of the canvas overlay
	void after_finish(cgv::render::context&) override;
	virtual void draw_content(cgv::render::context& ctx) = 0;

	void post_damage(bool redraw = true);
};

typedef cgv::data::ref_ptr<canvas_overlay> canvas_overlay_ptr;

}
}

#include <cgv/config/lib_end.h>
