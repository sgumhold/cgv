#include <cgv/base/base.h>
#include <cgv/base/node.h>
#include <cgv/render/drawable.h>
#include <cgv/base/find_action.h>

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

cgv::render::view* drawable::find_view_as_node(size_t view_idx) const
{
	cgv::base::node* node_ptr = const_cast<cgv::base::node*>(dynamic_cast<const cgv::base::node*>(this));
	std::vector<cgv::render::view*> views;
	cgv::base::find_interface<cgv::render::view>(cgv::base::base_ptr(node_ptr), views);
	if (views.empty() || view_idx > views.size())
		return 0;
	return views[view_idx];
}

bool drawable::get_world_location(int x, int y, const cgv::render::view& V, cgv::math::fvec<double, 3>& world_location, double* window_z_ptr) const
{
	if (!get_context())
		return false;
	// analyze the mouse location
	cgv::render::context& ctx = *get_context();
	const dmat4* DPV_ptr, *DPV_other_ptr;
	int x_other, y_other, vp_col_idx, vp_row_idx, vp_width, vp_height;
	int eye_panel = V.get_modelview_projection_window_matrices(x, y, ctx.get_width(), ctx.get_height(), &DPV_ptr, &DPV_other_ptr, &x_other, &y_other, &vp_col_idx, &vp_row_idx, &vp_width, &vp_height);

	// get the possibly two (if stereo is enabled) different device z-values
	double z = ctx.get_window_z(x, y);
	double z_other = ctx.get_window_z(x_other, y_other);
	//  unproject to world coordinates with smaller (closer to eye) z-value one	
	if (z <= z_other) {
		if (DPV_ptr->ncols() != 4)
			return false;
		// use conversion to (double*) operator to map cgv::math::vec<double> to cgv::math::fvec<float,3>
		world_location = ctx.get_model_point(x, y, z, *DPV_ptr);
		if (window_z_ptr)
			*window_z_ptr = z;
	}
	else {
		if (DPV_other_ptr->ncols() != 4)
			return false;
		world_location = ctx.get_model_point(x_other, y_other, z_other, *DPV_other_ptr);
		if (window_z_ptr)
			*window_z_ptr = z_other;
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

/// construct to be not inside of a render pass
multi_pass_drawable::multi_pass_drawable()
{
	current_render_pass = -1;
	render_pass_recursion_depth = 0;
}
/// call in init_frame method to check whether the recursive render passes need to be initiated
bool multi_pass_drawable::initiate_render_pass_recursion(context& ctx)
{
	if (current_render_pass != -1)
		return false;
	render_pass_recursion_depth = ctx.get_render_pass_recursion_depth();
	return true;
}
/// call to initiate a render pass in the init_frame method after initiate_render_pass_recursion() has succeeded
void multi_pass_drawable::perform_render_pass(context& ctx, int rp_idx, RenderPass rp, int excluded_flags, int included_flags)
{
	current_render_pass = rp_idx;
	unsigned rpf = (ctx.get_render_pass_flags() & ~excluded_flags) | included_flags;
	ctx.render_pass(rp, RenderPassFlags(rpf), this);
}
/// call after last recursive render pass to use current render pass for last render pass
void multi_pass_drawable::initiate_terminal_render_pass(int rp_idx)
{
	current_render_pass = rp_idx;
}
/// check in after_finish method, whether this should be directly exited with a return statement
bool multi_pass_drawable::multi_pass_ignore_finish(const context& ctx)
{
	if (current_render_pass == -1)
		return true;
	if (ctx.get_render_pass_user_data() != this)
		if (render_pass_recursion_depth != ctx.get_render_pass_recursion_depth())
			return true;
	return false;
}
/// check in after_finish method, whether this was the terminating render pass
bool multi_pass_drawable::multi_pass_terminate(const context& ctx)
{
	if (render_pass_recursion_depth != ctx.get_render_pass_recursion_depth())
		return false;
	current_render_pass = -1;
	return true;
}


	}
}
