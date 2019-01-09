#include <cgv/render/drawable.h>

namespace cgv {
	namespace render {

void drawable::set_context(context* _ctx)
{
	ctx = _ctx;
}
/// default construction
drawable::drawable() : base::traverse_policy(base::TP_ALL)
{
	ctx = 0;
}


/// hide the drawable
void drawable::hide()
{
	active = false;
}
/// show the drawable
void drawable::show()
{
	active = true;
}
/// check whether the drawable is visible
bool drawable::is_visible() const
{
	return active;
}

/// access the current context. The context will be available latestly in the init method but not in the contructor.
context* drawable::get_context() const
{
	return ctx;
}
/// posts a redraw event to the current context if one is available
void drawable::post_redraw()
{
	if (ctx)
		ctx->post_redraw();
}

bool drawable::get_world_location(int x, int y, const cgv::render::view& V, cgv::math::fvec<double, 3>& world_location) const
{
	if (!get_context())
		return false;
	// analyze the mouse location
	cgv::render::context& ctx = *get_context();
	const dmat4* DPV_ptr, *DPV_other_ptr;
	int x_other, y_other, vp_col_idx, vp_row_idx, vp_width, vp_height;
	int eye_panel = V.get_modelview_projection_device_matrices(x, y, ctx.get_width(), ctx.get_height(), &DPV_ptr, &DPV_other_ptr, &x_other, &y_other, &vp_col_idx, &vp_row_idx, &vp_width, &vp_height);

	// get the possibly two (if stereo is enabled) different device z-values
	double z = ctx.get_z_D(x, y);
	double z_other = ctx.get_z_D(x_other, y_other);
	//  unproject to world coordinates with smaller (closer to eye) z-value one	
	if (z <= z_other) {
		if (DPV_ptr->ncols() != 4)
			return false;
		// use conversion to (double*) operator to map cgv::math::vec<double> to cgv::math::fvec<float,3>
		world_location = ctx.get_point_W(x, y, z, *DPV_ptr);
	}
	else {
		if (DPV_other_ptr->ncols() != 4)
			return false;
		world_location = ctx.get_point_W(x_other, y_other, z_other, *DPV_other_ptr);
	}
	return true;
}

/// forces a redraw right now. This cannot be called from init, init_frame, draw, finish_draw, finish_frame and clear
void drawable::force_redraw()
{
	if (ctx)
		ctx->force_redraw();
}

/// this method is called after creation or recreation of the context, return whether all necessary functionality is supported
bool drawable::init(context&)
{
	return true;
}
/// callback to anounce resizing of the output window
void drawable::resize(unsigned int, unsigned int)
{
}
/// this method is called in one pass over all drawables before the draw method
void drawable::init_frame(context&)
{
}
/// overload to draw the content of this drawable
void drawable::draw(context&)
{
}
/// this method is called when the current drawable is left in a tree traversal that calls the draw method
void drawable::finish_draw(context&)
{
}
/// this method is called in one pass over all drawables after drawing
void drawable::finish_frame(context&)
{
}

/// this method is called in one pass over all drawables after finish frame
void drawable::after_finish(context&)
{
}

/// clear all objects living in the context like textures or display lists
void drawable::clear(context&)
{
}

	}
}
