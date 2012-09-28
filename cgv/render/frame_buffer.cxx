#include "frame_buffer.h"

namespace cgv {
	namespace render {

frame_buffer::frame_buffer()
{
	width = -1;
	height = -1;
}

/// destructor
frame_buffer::~frame_buffer()
{
	if (handle) {
		if (ctx_ptr)
			destruct(*ctx_ptr);
		else
			std::cerr << "could not destruct frame buffer properly." << std::endl;
	}
}

/// destruct the framebuffer objext
void frame_buffer::destruct(context& ctx)
{
	if (handle)
		ctx.frame_buffer_destruct(*this);
}

/// create framebuffer of extension is supported, otherwise return false.
bool frame_buffer::create(context& ctx, int _width, int _height)
{
	if (ctx_ptr)
		destruct(*ctx_ptr);
	else
		destruct(ctx);

	ctx_ptr = &ctx;
	width = _width;
	height = _height;
	return ctx.frame_buffer_create(*this);
}

void frame_buffer::set_width(int _width)
{
	width = _width;
}

void frame_buffer::set_height(int _height)
{
	height = _height;
}

/// set different size
void frame_buffer::set_size(int _width, int _height)
{
	width = _width;
	height = _height;
}

/// attach render buffer to depth buffer if it is a depth buffer, to stencil if it is a stencil buffer or to the i-th color attachment if it is a color buffer
bool frame_buffer::attach(context& ctx, const render_buffer& rb, int i)
{
	return ctx.frame_buffer_attach(*this, rb, rb.is_depth_buffer(), i);
}

/// attach 2d texture to depth buffer if it is a depth texture, to stencil if it is a stencil texture or to the i-th color attachment if it is a color texture
bool frame_buffer::attach(context& ctx, const texture& tex2d, int level, int i)
{
	if (tex2d.get_nr_dimensions() != 2) {
		last_error = "can only attach 2d textures to a frame buffer";
		return false;
	}
	tex2d.ensure_state(ctx);
	return ctx.frame_buffer_attach(*this, tex2d,
		tex2d.get_standard_component_format() == cgv::data::CF_D, level, i, -1);
}
/// attach the j-th slice of a 3d texture to the i-th color attachment
bool frame_buffer::attach(context& ctx, const texture& tex3d, int z_or_cube_side, int level, int i)
{
	if (tex3d.get_nr_dimensions() != 3) {
		last_error = "can only attach slices of 3d textures to a frame buffer";
		return false;
	}
	tex3d.ensure_state(ctx);
	return ctx.frame_buffer_attach(*this, tex3d,
		tex3d.get_standard_component_format() == cgv::data::CF_D, level, i, z_or_cube_side);
}

/// check for completeness, if not complete, get the reason in last_error
bool frame_buffer::is_complete(context& ctx) const
{
	return ctx.frame_buffer_is_complete(*this);
}

/** enable the framebuffer either with all color attachments 
    if no arguments are given or if arguments are given with 
	 the indexed color attachments. Return whether this was successful. */
bool frame_buffer::enable(context& ctx, int i0, int i1, int i2, int i3
							, int i4 , int i5 , int i6 , int i7 
							, int i8 , int i9 , int i10, int i11
							, int i12, int i13, int i14, int i15)
{
	enabled_color_attachments.clear();
	if (i0 >= 0)
		enabled_color_attachments.push_back(i0);
	if (i1 >= 0)
		enabled_color_attachments.push_back(i1);
	if (i2 >= 0)
		enabled_color_attachments.push_back(i2);
	if (i3 >= 0)
		enabled_color_attachments.push_back(i3);
	if (i4 >= 0)
		enabled_color_attachments.push_back(i4);
	if (i5 >= 0)
		enabled_color_attachments.push_back(i5);
	if (i6 >= 0)
		enabled_color_attachments.push_back(i6);
	if (i7 >= 0)
		enabled_color_attachments.push_back(i7);
	if (i8 >= 0)
		enabled_color_attachments.push_back(i8);
	if (i9 >= 0)
		enabled_color_attachments.push_back(i9);
	if (i10>= 0)
		enabled_color_attachments.push_back(i10);
	if (i11>= 0)
		enabled_color_attachments.push_back(i11);
	if (i12>= 0)
		enabled_color_attachments.push_back(i12);
	if (i13>= 0)
		enabled_color_attachments.push_back(i13);
	if (i14>= 0)
		enabled_color_attachments.push_back(i14);
	if (i15>= 0)
		enabled_color_attachments.push_back(i15);
	return ctx.frame_buffer_enable(*this);
}

bool frame_buffer::enable(context& ctx, std::vector<int>& indices)
{
	enabled_color_attachments = indices;
	return ctx.frame_buffer_enable(*this);
}


/// disable the framebuffer object
bool frame_buffer::disable(context& ctx)
{
	return ctx.frame_buffer_disable(*this);
}

int frame_buffer::get_max_nr_color_attachments(context& ctx)
{
	return ctx.frame_buffer_get_max_nr_color_attachments();
}

int frame_buffer::get_max_nr_draw_buffers(context& ctx)
{
	return ctx.frame_buffer_get_max_nr_draw_buffers();
}


	}
}