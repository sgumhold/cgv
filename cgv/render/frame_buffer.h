#pragma once

#include <cgv/data/component_format.h>
#include <cgv/render/texture.h>
#include <cgv/render/render_buffer.h>
#include <cgv/render/context.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {

/** this class encapsulate frame buffers that live on the GPU
    and can be used as destination for the render process.
	 The GPU must support frame buffer objects for this to work. */
class CGV_API frame_buffer : public frame_buffer_base
{
public:
	/// return the maximum number of color attachments supported by the GPU
	static int get_max_nr_color_attachments(const context& ctx);
	/// return the maximum number of simultaneous draw buffers supported by the GPU
	static int get_max_nr_draw_buffers(const context& ctx);
	/// a string that contains the last error, which is only set by the init method
	mutable std::string last_error;
	/// constructor just initializes stuff
	frame_buffer();
	/// destructor
	~frame_buffer();
	/// destruct the framebuffer objext
	void destruct(const context& ctx);
	/** create framebuffer if extension is supported, otherwise return false. 
	    The set extent is used to set the viewport in the enable method. 
		 If no extent is specified it is copied from glViewport. */
	bool create(const context& ctx, int _width = -1, int _height = -1);
	/// set a different width
	void set_width(int _width);
	/// set a different height
	void set_height(int _height);
	/// set different size
	void set_size(int _width, int _height);
	/// return the width
	int get_width() const { return width; }
	/// return the height
	int get_height() const { return height; }

	/// attach render buffer to depth buffer if it is a depth buffer, to stencil if it is a stencil buffer or to the i-th color attachment if it is a color buffer
	bool attach(const context& ctx, const render_buffer& rb, int i = 0);
	/// attach 2d texture to depth buffer if it is a depth texture, to stencil if it is a stencil texture or to the i-th color attachment if it is a color texture
	bool attach(const context& ctx, const texture& tex2d, int level = 0, int i = 0);
	/// attach the j-th slice of a 3d texture or the given cube side of a cubemap to the i-th color attachment
	bool attach(const context& ctx, const texture& tex3d, int z_or_cube_side, int level, int i);
	/// check for completeness, if not complete, get the reason in last_error
	bool is_complete(const context& ctx) const;
	/** enable the framebuffer either with all color attachments 
	    if no arguments are given or if arguments are given with 
		 the indexed color attachments. Return whether this was successful. */
	bool enable(context& ctx, int i0 = -1, int i1 = -1, int i2 = -1, int i3 = -1
							, int i4 = -1, int i5 = -1, int i6 = -1, int i7 = -1
							, int i8 = -1, int i9 = -1, int i10= -1, int i11= -1
							, int i12= -1, int i13= -1, int i14= -1, int i15= -1);
	/** enable the framebuffer either with the color attachments 
	    specified in the given vector. */
	bool enable(context& ctx, std::vector<int>& indices);
	/// push a new window transformation to cover the fbo onto the window transformation stack
	void push_viewport(context& ctx, const cgv::render::render_types::dvec2& depth_range = cgv::render::render_types::dvec2(0,1));
	/// recover the window transformation array active before the last call to push_viewport
	void pop_viewport(context& ctx);
	/// disable the framebuffer object
	bool disable(context& ctx);
	/// different variants of blitting rectangle src_rect of this frame buffer into rectangle dest_rect of destination frame buffer
	void blit_to(context& ctx, BufferTypeBits btbs = BTB_ALL_BITS, bool interpolate = false) const { blit_to(ctx, ivec4(0, 0, width, height), ivec4(-1), btbs, interpolate); }
	void blit_to(context& ctx, const ivec4& src_rect, BufferTypeBits btbs = BTB_ALL_BITS, bool interpolate = false) const { blit_to(ctx, src_rect, ivec4(-1), btbs, interpolate); }
	void blit_to(context& ctx, const ivec4& src_rect, const ivec4& dst_rect, BufferTypeBits btbs = BTB_ALL_BITS, bool interpolate = false) const;
	void blit_to(context& ctx, frame_buffer& dst_fbo, BufferTypeBits btbs = BTB_ALL_BITS, bool interpolate = false) const { blit_to(ctx, dst_fbo, ivec4(0, 0, width, height), ivec4(0, 0, dst_fbo.width, dst_fbo.height), btbs, interpolate); }
	void blit_to(context& ctx, frame_buffer& dst_fbo, const ivec4& src_rect, BufferTypeBits btbs = BTB_ALL_BITS, bool interpolate = false) const { blit_to(ctx, dst_fbo, src_rect, ivec4(0, 0, dst_fbo.width, dst_fbo.height), btbs, interpolate); }
	void blit_to(context& ctx, frame_buffer& dst_fbo, const ivec4& src_rect, const ivec4& dst_rect, BufferTypeBits btbs = BTB_ALL_BITS, bool interpolate = false) const;
	/// different variants of blitting to rectangle dst_rect of this frame buffer from rectangle src_rect of source frame buffer
	void blit_from(context& ctx, BufferTypeBits btbs = BTB_ALL_BITS, bool interpolate = false) { blit_from(ctx, ivec4(0, 0, width, height), ivec4(-1), btbs, interpolate); }
	void blit_from(context& ctx, const ivec4& dst_rect, BufferTypeBits btbs = BTB_ALL_BITS, bool interpolate = false) { blit_from(ctx, dst_rect, ivec4(-1), btbs, interpolate); }
	void blit_from(context& ctx, const ivec4& dst_rect, const ivec4& src_rect, BufferTypeBits btbs = BTB_ALL_BITS, bool interpolate = false);
	void blit_from(context& ctx, frame_buffer& src_fbo, BufferTypeBits btbs = BTB_ALL_BITS, bool interpolate = false) { blit_from(ctx, src_fbo, ivec4(0, 0, width, height), ivec4(0, 0, src_fbo.width, src_fbo.height), btbs, interpolate); }
	void blit_from(context& ctx, frame_buffer& src_fbo, const ivec4& dst_rect, BufferTypeBits btbs = BTB_ALL_BITS, bool interpolate = false) { blit_to(ctx, src_fbo, dst_rect, ivec4(0, 0, src_fbo.width, src_fbo.height), btbs, interpolate); }
	void blit_from(context& ctx, frame_buffer& src_fbo, const ivec4& dst_rect, const ivec4& src_rect, BufferTypeBits btbs = BTB_ALL_BITS, bool interpolate = false);
};

	}
}

#include <cgv/config/lib_end.h>

