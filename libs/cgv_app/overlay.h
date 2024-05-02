#pragma once

#include <cgv/defines/deprecated.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/context.h>
#include <cgv/render/drawable.h>
#include <cgv/utils/pointer_test.h>
#include <cgv_g2d/trect.h>
#include <cgv_g2d/utils2d.h>

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
		AO_PERCENTUAL,	// alignment based on offset from lower left corner based on percentual size of viewport
	};

	enum StretchOption {
		SO_NONE,		// size is unchanged
		SO_HORIZONTAL,	// width is stretched to cover viewport
		SO_VERTICAL,	// height is stretched to cover viewport
		SO_BOTH,		// width and height are stretched to cover viewport
		SO_PERCENTUAL,  // width and height are scaled according to percentual size of viewport
	};

private:
	/// the last recorded size of the viewport, is kept current with ensure_viewport
	ivec2 last_viewport_size_ = ivec2(-1);
	/// the last recorded size of this overlay
	ivec2 last_size_ = ivec2(-1);
	/// rectangle area this overlay is fully contained whithin
	cgv::g2d::irect container_;

	/// layout parameters
	AlignmentOption horizontal_alignment_ = AlignmentOption::AO_START;
	AlignmentOption vertical_alignment_ = AlignmentOption::AO_START;
	StretchOption stretch_ = SO_NONE;
	ivec2 margin_ = ivec2(0);
	vec2 percentual_offset_ = vec2(0.0f);
	vec2 percentual_size_ = vec2(1.0f);

	template<typename T>
	data::ref_ptr<cgv::gui::control<T>> add_layout_member_control(const std::string& label, T& value, const std::string& gui_type = "", const std::string& options = "", const std::string& align = "\n") {
		data::ref_ptr<cgv::gui::control<T>> cp = add_control(label, value, gui_type, options, align);
		if(cp)
			connect_copy(cp->value_change, cgv::signal::rebind(this, &overlay::on_layout_change));
		return cp;
	}

protected:
	/// whether the overlay blocks events or lets them pass through to other handlers (by default only derived classes may set this property)
	bool block_events = false;
	
	/// called when the overlay visibility is changed through the default gui
	virtual void on_visibility_change();

	/// called when the overlay layout parameters are changed through the default gui
	virtual void on_layout_change();

	/// update the layout of the overlay container
	void update_layout();

	/// virtual method to implement the derived class gui creation
	virtual void create_gui_impl() {};

public:
	struct gui_options_t {
		std::string heading = "";
		/// if true the overlay GUI will be placed inside a collapsible tree node
		bool create_default_tree_node = true;
		/// whether to show the overlay name as a heading
		bool show_heading = false;
		/// whether to show the layout options
		bool show_layout_options = true;
		/// whether to show the alignment options (show_layout_options must be enabled)
		bool allow_alignment = true;
		/// whether to show the stretch options (show_layout_options must be enabled)
		bool allow_stretch = true;
		/// whether to show the margin options (show_layout_options must be enabled)
		bool allow_margin = true;
	};
	/// options for the GUI creation of this overlay (must be set before GUI creation)
	gui_options_t gui_options;

	/// overload to reflect members of derived classes
	virtual bool self_reflect(cgv::reflect::reflection_handler& _rh) { return false; }

	/// overload to stream help information to the given output stream
	virtual void stream_help(std::ostream& os) {};

	/// default implementation of handle method returns false, use handle_events instead
	virtual bool handle(cgv::gui::event& e) { return false; };

	/// overload this method to handle events
	virtual bool handle_event(cgv::gui::event& e) { return false; };

	/// implement to handle member changes
	virtual void handle_member_change(const cgv::utils::pointer_test& m) {}

	/// default implementation of that calls handle_member_change and afterwards updates the member in the gui and post a redraw
	virtual void on_set(void* member_ptr);

	/// return whether this overlay blocks events, i.e. does not pass them to the next event handler
	bool blocks_events() const { return block_events; }

	/// return the current viewport size
	ivec2 get_viewport_size() const { return last_viewport_size_; }

	/// return the mouse position local to the container of this overlay
	ivec2 get_local_mouse_pos(ivec2 mouse_pos) const;

	/// return the current rectangle area (in screen coordinates) of the overlay taking layout into account
	cgv::g2d::irect get_rectangle() const { return container_; }

	/// return the current rectangle area of the overlay in local space, i.e. with position set to zero
	cgv::g2d::irect get_local_rectangle() const { return cgv::g2d::irect(ivec2(0), container_.size); }

	/// get the horizontal alignment
	AlignmentOption get_horizontal_alignment() const { return horizontal_alignment_; }

	/// get the vertical alignment
	AlignmentOption get_vertical_alignment() const { return vertical_alignment_; }

	/// get the percentual alignment offset (only valid if get_horizontal_alignment() or get_vertical_alignment() returns AlignmentOption::AO_PERCENTUAL)
	vec2 get_percentual_offset() const { return percentual_offset_; }

	/// set the alignment options
	void set_alignment(AlignmentOption horizontal, AlignmentOption vertical, vec2 percentual_offset = vec2(-1.0f));

	/// get the stretch
	StretchOption get_stretch() const { return stretch_; }

	/// get the percentual stretch (only valid if get_stretch() returns StretchOption::SO_PERCENTUAL)
	vec2 get_percentual_size() const { return percentual_size_; }

	/// set the stretch option
	void set_stretch(StretchOption stretch, vec2 percentual_size = vec2(-1.0f));

	/// return the margin as set in the layout parameters
	ivec2 get_margin() const { return margin_; }

	/// set the overlay margin
	void set_margin(const ivec2& margin);

	/// set the default size of the overlay before stretch gets applied
	void set_size(const ivec2& size);

	/// set the visibility of the overlay
	void set_visibility(bool visible);

	/// toggle the visibility of the overlay
	void toggle_visibility();

	/** Check whether the viewport size has changed since the last call to
		this method. Call this in the init_frame method of your overlay.
		Returns true if the size changed.
	*/
	bool ensure_viewport(cgv::render::context& ctx);

	bool ensure_layout(cgv::render::context& ctx);

	/** Test if the mouse pointer is hovering over this overlay and returns
		true if this is the case. Specifically it checks if the mouse position
		is inside the rectangle defined by container. Override this method to
		implement your own test, i.e. for different overlay shapes.
	*/
	virtual bool is_hit(const ivec2& mouse_pos) const;

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
