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
public:
	/// declare rgb color type
	typedef cgv::media::color<float, cgv::media::RGB> rgba_type;
	/// declare rgba color type
	typedef cgv::media::color<float, cgv::media::RGB, cgv::media::OPACITY> rgb_type;
	/// declare type of 2d vectors
	typedef cgv::math::fvec<float, 2> vec2;
	/// declare type of 3d vectors
	typedef cgv::math::fvec<float, 3> vec3;
	/// declare type of homogeneous vectors
	typedef cgv::math::fvec<float, 4> vec4;
	/// declare type of 2x2 matrices
	typedef cgv::math::fmat<float, 2, 2> mat2;
	/// declare type of 3x3 matrices
	typedef cgv::math::fmat<float, 3, 3> mat3;
	/// declare type of 4x4 matrices
	typedef cgv::math::fmat<float, 4, 4> mat4;
	/// declare type of 2d vectors
	typedef cgv::math::fvec<double, 2> dvec2;
	/// declare type of 3d vectors
	typedef cgv::math::fvec<double, 3> dvec3;
	/// declare type of homogeneous vectors
	typedef cgv::math::fvec<double, 4> dvec4;
	/// declare type of 2x2 matrices
	typedef cgv::math::fmat<double, 2, 2> dmat2;
	/// declare type of 3x3 matrices
	typedef cgv::math::fmat<double, 3, 3> dmat3;
	/// declare type of 4x4 matrices
	typedef cgv::math::fmat<double, 4, 4> dmat4;
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