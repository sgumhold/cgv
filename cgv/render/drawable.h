#pragma once

#include <cgv/base/base.h>
#include <cgv/base/traverser.h>
#include <cgv/render/context.h>
#include <cgv/render/view.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {

/** base class for all drawables, which is independent of the used rendering API. */
class CGV_API drawable : public base::traverse_policy, public render_types
{
private:
	/// store the context
	context* ctx;
public:
	/// default construction
	drawable();

	/// hide the drawable
	void hide();
	/// show the drawable
	void show();
	/// check whether the drawable is visible
	bool is_visible() const;

	/// access the current context. The context will be available latestly in the init method but not in the contructor.
	context* get_context() const;
	/// set the current focus context, this should only be called by the context itself
	void set_context(context* _ctx);
	//! convenience function to find the view control in the current hierarchy
	/*! this only works if your class inherits from the cgv::base::node class.*/
	cgv::render::view* find_view_as_node(size_t view_idx = 0) const;
	//! use given view together with depth buffer of context in order to compute the world location of the point at mouse pointer location (x,y)
	/*! returns true if a world location could be computed which is the case when the context pointer of the drawable has been set and when 
	    the mouse location points inside a valid view panel. */
	bool get_world_location(int x, int y, const cgv::render::view& V, cgv::math::fvec<double, 3>& world_location, double* window_z_ptr = 0) const;
	/// posts a redraw event to the current context if one is available
	void post_redraw();
	/// forces a redraw right now. This cannot be called from init, init_frame, draw, finish_draw, finish_frame and clear
	void force_redraw();

	/// this method is called after creation or recreation of the context, return whether all necessary functionality is supported
	virtual bool init(context&);
	/// callback to anounce resizing of the output window
	virtual void resize(unsigned int w, unsigned int h);
	/// this method is called in one pass over all drawables before the draw method
	virtual void init_frame(context&);
	/// overload to draw the content of this drawable
	virtual void draw(context&);
	/// this method is called when the current drawable is left in a tree traversal that calls the draw method
	virtual void finish_draw(context&);
	/// this method is called in one pass over all drawables after drawing
	virtual void finish_frame(context&);
	/// this method is called in one pass over all drawables after finish frame
	virtual void after_finish(cgv::render::context&);
	/// clear all objects living in the context like textures or display lists
	virtual void clear(context&);
};

/** helper class to control multiple render passes in init_frame and after_finish methods of drawable.  */
class CGV_API multi_pass_drawable : public drawable
{
protected:
	// mark current render pass
	int current_render_pass;
	// store recursion depth on which render passes are initiated
	unsigned render_pass_recursion_depth;
public:
	/// construct to be not inside of a render pass
	multi_pass_drawable();
	/// call in init_frame method to check whether the recursive render passes need to be initiated
	bool initiate_render_pass_recursion(context& ctx);
	/// call to initiate a render pass in the init_frame method after initiate_render_pass_recursion() has succeeded
	void perform_render_pass(context& ctx, int rp_idx, RenderPass rp = RP_USER_DEFINED, int excluded_flags = RPF_HANDLE_SCREEN_SHOT, int included_flags = 0);
	/// call after last recursive render pass to use current render pass for last render pass
	void initiate_terminal_render_pass(int rp_idx);
	/// check in after_finish method, whether this should be directly exited with a return statement
	bool multi_pass_ignore_finish(const context& ctx);
	/// check in after_finish method, whether this was the terminating render pass
	bool multi_pass_terminate(const context& ctx);
};

	}
}

#include <cgv/config/lib_end.h>