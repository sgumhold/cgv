#pragma once

#include <cgv_app/canvas_overlay.h>
#include <cgv/gui/theme_info.h>

#include "lib_begin.h"

namespace cgv {
namespace app {

class CGV_API themed_canvas_overlay : public canvas_overlay, public cgv::gui::theme_observer {
private:
	int padding_ = 8;

	cgv::g2d::shape2d_style container_style_;

protected:
	/// whether the background is visible (true by default)
	bool background_visible = true;

	void begin_content(cgv::render::context& ctx, bool clear_frame_buffer = true);
	
	virtual void init_styles() {}

	void init_container_style(const cgv::gui::theme_info& theme);

	/// return the current rectangle area of the themed_overlay content
	cgv::g2d::irect get_content_rectangle() const;

	int padding() const { return padding_; }

public:
	/// creates an overlay in the bottom left corner with zero size using a canvas for 2d drawing
	themed_canvas_overlay();

	bool init(cgv::render::context& ctx) override;

	virtual void handle_theme_change(const cgv::gui::theme_info& theme) override;
};

typedef cgv::data::ref_ptr<canvas_overlay> canvas_overlay_ptr;

}
}

#include <cgv/config/lib_end.h>
