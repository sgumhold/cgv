#pragma once

#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/context.h>
#include <cgv/render/drawable.h>

#include "2d/rect.h"

#include "lib_begin.h"

namespace cgv {
namespace glutil {

class CGV_API overlay :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider,
	public cgv::gui::event_handler
{
public:
	enum AlignmentOption {
		AO_FREE,	// alignment solely controlled by margin
		AO_START,	// left for horizontal, bottom for vertical direction
		AO_CENTER,	// center of the viewport for both directions
		AO_END,		// right for horizontal, top for vertical direction
	};

	enum StretchOption {
		SO_NONE,		// size is unchanged
		SO_HORIZONTAL,	// width is stretched to cover viewport
		SO_VERTICAL,	// height is stretched to cover viewport
		SO_BOTH			// width and height are stretched to cover viewport
	};

protected:
	/// the last recorded size of the viewport, is kept current with ensure_viewport
	ivec2 last_viewport_size;

	/// layout parameters
	AlignmentOption horizontal_alignment;
	AlignmentOption vertical_alignment;
	StretchOption stretch;
	ivec2 margin;
	ivec2 size;
	bool show; /// whether the overlay is visible
	bool block_events; /// whether the overlay blocks events or lets them pass through to other handlers

	/// rectangle area of this overlay is fully contained whithin
	rect container;
	ivec2 last_size;

	/// a pointer to the parent event handler
	cgv::gui::event_handler* parent_handler = nullptr;

	/// called when the overlay visibility is changed through the default gui
	void on_visibility_change();

	/// called when the overlay layout parameters are changed through the default gui
	void on_layout_change();

	/// updates the layout of the overlay container
	void update_overlay_layout();

public:
	struct {
		bool show_heading = true;
		bool show_layout_options = true;
		bool allow_alignment = true;
		bool allow_stretch = true;
		bool allow_margin = true;
	} gui_options;

	/// creates an overlay in the bottom left corner with zero size
	overlay();

	/// finalize the handle method to prevent overloading in implementations of this class, use handle_events instead
	virtual bool handle(cgv::gui::event& e) final {
		return parent_handler->handle(e);
	};
	/// overload to stream help information to the given output stream
	virtual void stream_help(std::ostream& os) {};

	/// sets the parent event handler which gets called in the handle method (the parent shall always handle events first)
	void set_parent_handler(cgv::gui::event_handler* parent_handler) {
		this->parent_handler = parent_handler;
	}

	/// overload this method to handle events
	virtual bool handle_event(cgv::gui::event& e) { return false; };

	bool blocks_events() { return block_events; }

	/// returns the current viewport size
	ivec2 get_viewport_size() {
		return last_viewport_size;
	}

	/** Returns the mouse position transformed from FLTK window to OpenGL
		viewport spaceThe OpenGL viewport origin is in the bottom left
		while the FLTK origin is in the top left.
	*/
	ivec2 get_transformed_mouse_pos(ivec2 mouse_pos);

	/** Returns the mouse position local to the container of this overlay.
	*/
	ivec2 get_local_mouse_pos(ivec2 mouse_pos);

	/// sets the alignment options
	void set_overlay_alignment(AlignmentOption horizontal, AlignmentOption vertical) {
		horizontal_alignment = horizontal;
		vertical_alignment = vertical;
		update_overlay_layout();
	}

	/// sets the stretch option
	void set_overlay_stretch(StretchOption stretch) {
		this->stretch = stretch;
		update_overlay_layout();
	}

	/// returns the position of the overlay with origin at the bottom left
	ivec2 get_overlay_position() { return container.pos(); }

	/// returns the margin as set in the layout parameters
	ivec2 get_overlay_margin() { return margin; }

	/// sets the overlay margin
	void set_overlay_margin(const ivec2& m) {
		margin = m;
		update_overlay_layout();
	}

	/// returns the current size of the overlay taking strech parameters into account
	ivec2 get_overlay_size() { return container.size(); }

	/// sets the default size of the overlay before stretch gets applied
	void set_overlay_size(const ivec2& s) {
		size = s;
		update_overlay_layout();
	}

	/// return the visibility state of the overlay
	bool is_visible() { return show; }

	/// sets the visibility of the overlay to flag
	void set_visibility(bool flag) {
		show = flag;
		update_member(&show);
	}

	/** Checks whether the viewport size has changed since the last call to
		this method. Call this in the init_frame method of your overlay.
		Returns true if the size changed.
	*/
	bool ensure_viewport(cgv::render::context& ctx);

	bool ensure_overlay_layout(cgv::render::context& ctx) {
		bool ret = ensure_viewport(ctx);

		/*ivec2 viewport_size(ctx.get_width(), ctx.get_height());
		if(last_viewport_size != viewport_size) {
			last_viewport_size = viewport_size;
			update_overlay_layout();
			ret = true;
		}*/

		ivec2 current_size = container.size();
		if(last_size != current_size) {
			last_size = current_size;
			ret = true;
		}

		return ret;
	}

	/** Tests if the mouse pointer is hovering over this overlay and returns
		true if this is the case. Specifically it checks if the mouse position
		is inside the rectangle defined by container. Override this method to
		implement your own check, i.e. for different overlay shapes.
	*/
	virtual bool is_hit(const ivec2& mouse_pos);

	/// provides a default gui implementation for private overlay layout members
	void create_overlay_gui();
};

}
}

#include <cgv/config/lib_end.h>
