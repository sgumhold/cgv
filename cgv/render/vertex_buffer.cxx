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

void vertex_buffer::bind(context& ctx, VertexBufferType _type) const
{
	ctx.vertex_buffer_bind(*this, _type == VBT_UNDEF ? this->type : _type);
}

void vertex_buffer::unbind(context& ctx, VertexBufferType _type) const
{
	ctx.vertex_buffer_unbind(*this, _type == VBT_UNDEF ? this->type : _type);
}

void vertex_buffer::unbind(context& ctx, unsigned index) const { ctx.vertex_buffer_unbind(*this, this->type, index); }

void vertex_buffer::unbind(context& ctx, VertexBufferType type, unsigned index) const
{
	ctx.vertex_buffer_unbind(*this, type, index);
}

void vertex_buffer::bind(context& ctx, unsigned index) const
{
	ctx.vertex_buffer_bind(*this, this->type, index);
}

void vertex_buffer::bind(context& ctx, VertexBufferType _type, unsigned index) const
{
	ctx.vertex_buffer_bind(*this, _type, index);
}

/// create empty vertex buffer of size \c size given in bytes
bool vertex_buffer::create(const context& ctx, size_t _size_in_bytes)
{
	size_in_bytes = _size_in_bytes;
	return ctx.vertex_buffer_create(*this, 0, size_in_bytes);
}

bool vertex_buffer::is_created() const
{
	return handle != 0;
}

bool vertex_buffer::resize(const context& ctx, size_t size_in_bytes)
{
	this->size_in_bytes = size_in_bytes;
	return ctx.vertex_buffer_resize(*this, 0, size_in_bytes);
}

bool vertex_buffer::create_or_resize(const context& ctx, size_t size_in_bytes)
{
	return !is_created() ? create(ctx, size_in_bytes) : resize(ctx, size_in_bytes);
}

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