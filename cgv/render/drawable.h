#pragma once

#include <cgv/base/base.h>
#include <cgv/base/traverser.h>
#include <cgv/render/context.h>
#include <cgv/render/view.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {

/** base class for all drawables, which is independent of the used rendering API. */
class CGV_API drawable : public base::traverse_policy
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
	//! use given view together with depth buffer of context in order to compute the world location of the point at mouse pointer location (x,y)
	/*! returns true if a world location could be computed which is the case when the context pointer of the drawable has been set and when 
	    the mouse location points inside a valid view panel. */
	bool get_world_location(int x, int y, const cgv::render::view& V, cgv::math::fvec<double, 3>& world_location) const;
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

	}
}

#include <cgv/config/lib_end.h>