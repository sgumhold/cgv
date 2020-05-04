#include "vertex_buffer.h"

namespace cgv {
	namespace render {

/// construct from description of component format, where the default format specifies a color buffer with alpha channel
vertex_buffer::vertex_buffer(VertexBufferType _type, VertexBufferUsage _usage)
{
	type = _type;
	usage = _usage;
	size_in_bytes = 0;
}

/// calls the destruct method if necessary
vertex_buffer::~vertex_buffer()
{
	if (ctx_ptr) {
		if (ctx_ptr->make_current()) {
			destruct(*ctx_ptr);
			ctx_ptr = 0;
		}
	}
}

size_t vertex_buffer::get_size_in_bytes() const
{
	return size_in_bytes;
}

/// bind buffer potentially overwriting buffer type
void vertex_buffer::bind(context& ctx, VertexBufferType _type) const
{
	ctx.vertex_buffer_bind(*this, _type == VBT_UNDEF ? this->type : _type);
}

/// create empty vertex buffer of size \c size given in bytes
bool vertex_buffer::create(const context& ctx, size_t _size_in_bytes)
{
	size_in_bytes = _size_in_bytes;
	return ctx.vertex_buffer_create(*this, 0, size_in_bytes);
}

/// check whether the vertex buffer has been created
bool vertex_buffer::is_created() const
{
	return handle != 0;
}

/// copy \c size_in_bytes number bytes from this vertex buffer starting at byte offset \c start_offset_in_bytes to vertex buffer \c dst starting at offest \c dst_offset_in_bytes
bool vertex_buffer::copy(const context& ctx, size_t src_offset_in_bytes, size_t size_in_bytes, vertex_buffer& dst, size_t dst_offset_in_bytes) const
{
	return ctx.vertex_buffer_copy(*this, src_offset_in_bytes, dst, dst_offset_in_bytes, size_in_bytes);
}

/// destruct the render buffer
void vertex_buffer::destruct(const context& ctx)
{
	if (handle) {
		ctx.vertex_buffer_destruct(*this);
		size_in_bytes = 0;
		handle = 0;
	}
}


	}
}