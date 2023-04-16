#pragma once

#include <cgv/defines/deprecated.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/context.h>
#include <cgv/render/drawable.h>
#include <cgv_g2d/rect.h>

#include "lib_begin.h"

namespace cgv {
namespace app {

class CGV_API overlay :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider,
	public cgv::gui::event_handler
{
public:
	enum AlignmentOption {
		AO_FREE,		// alignment solely controlled by margin
		AO_START,		// left for horizontal, bottom for vertical direction
		AO_CENTER,		// center of the viewport for both directions
		AO_END,			// right for horizontal, top for vertical direction
		AO_PERCENTUAL,	// use percentual offset
	};

	enum StretchOption {
		SO_NONE,		// size is unchanged
		SO_HORIZONTAL,	// width is stretched to cover viewport
		SO_VERTICAL,	// height is stretched to cover viewport
		SO_BOTH,		// width and height are stretched to cover viewport
		SO_PERCENTUAL,  // use percentual size
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
	vec2 percentual_offset = vec2(0.0f);
	vec2 percentual_size = vec2(1.0f);

	/// whether the overlay is visible
	bool show;
	/// whether the overlay blocks events or lets them pass through to other handlers
	bool block_events;

	/// rectangle area of this overlay is fully contained whithin
	cgv::g2d::irect container;
	ivec2 last_size;

	/// called when the overlay visibility is changed through the default gui
	virtual void on_visibility_change();

	/// called when the overlay layout parameters are changed through the default gui
	virtual void on_layout_change();

	/// updates the layout of the overlay container
	void update_overlay_layout();

	/// virtual method to implement the derived class gui creation
	virtual void create_gui_impl() {};

public:
	struct {
		std::string heading = "";
		bool create_default_tree_node = true;
		bool show_heading = false;
		bool show_layout_options = true;
		bool allow_alignment = true;
		bool allow_stretch = true;
		bool allow_margin = true;
	} gui_options;

	/// creates an overlay in the bottom left corner with zero size
	overlay();

	/// overload to reflect members of derived classes
	virtual bool self_reflect(cgv::reflect::reflection_handler& _rh) { return false; }

	/// overload to stream help information to the given output stream
	virtual void stream_help(std::ostream& os) {};

	/// finalize the handle method to prevent overloading in implementations of this class, use handle_events instead
	virtual bool handle(cgv::gui::event& e) final {
		return false;
	};

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
	void set_overlay_alignment(AlignmentOption horizontal, AlignmentOption vertical, vec2 _percentual_offset = vec2(-1.0f));

	/// sets the stretch option
	void set_overlay_stretch(StretchOption stretch, vec2 _percentual_size = vec2(-1.0f));

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
		on_visibility_change();
	}

	/// convenience method to toggle the visibility of the overlay
	void toggle_visibility() {
		set_visibility(!show);
	}

	/** Checks whether the viewport size has changed since the last call to
		this method. Call this in the init_frame method of your overlay.
		Returns true if the size changed.
	*/
	bool ensure_viewport(cgv::render::context& ctx);

	bool ensure_overlay_layout(cgv::render::context& ctx) {
		bool ret = ensure_viewport(ctx);

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

	/// begins a tree node if create default tree node is set in the gui options; automatically creates the layout gui
	bool begin_overlay_gui();
	/// ends the tree node of the overlay gui
	void end_overlay_gui();

	/// provides a default gui implementation for private overlay layout members
	void create_layout_gui();

	/** Creates a tree node containing the overlay layout options and the gui as specified by the derived overlay class.
		If you don't need to extend the gui of the overlay in your own plugin, call this method via:

			inline_object_gui(overlay_ptr)

		in your create_gui() method.

		If you want to extend the default gui inside the provided tree node:
		1. Set overlay_ptr.gui_options.create_default_tree_node = false
		2. In the plugin create_gui() method:

			integrate_object_gui(overlay_ptr);
			if(overlay_ptr->begin_overlay_gui()) {
				overlay_ptr->create_gui();
					
				...add custom controls
				
				overlay_ptr->end_overlay_gui();
			}

		!!! Do not overwrite create_gui() in the derived class if you want the default behaviour. !!!
	*/
	void create_gui();
};

typedef cgv::data::ref_ptr<overlay> overlay_ptr;

}
}

#include <cgv/config/lib_end.h>
