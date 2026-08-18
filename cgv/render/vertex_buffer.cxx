#include "vertex_buffer.h"

namespace cgv {
namespace render {

vertex_buffer::vertex_buffer(VertexBufferType type, VertexBufferUsage usage)
{
	this->type = type;
	this->usage = usage;
}

vertex_buffer::~vertex_buffer()
{
	if (ctx_ptr) {
		if (ctx_ptr->make_current()) {
			destruct(*ctx_ptr);
			ctx_ptr = nullptr;
		}
	}
}

size_t vertex_buffer::get_size_in_bytes() const
{
	return size_in_bytes;
}

void vertex_buffer::bind(const context& ctx, VertexBufferType type) const
{
	ctx.vertex_buffer_bind(*this, type == VBT_UNDEF ? this->type : type);
}

void vertex_buffer::unbind(const context& ctx, VertexBufferType type) const
{
	ctx.vertex_buffer_unbind(*this, type == VBT_UNDEF ? this->type : type);
}

void vertex_buffer::unbind(const context& ctx, unsigned index) const { ctx.vertex_buffer_unbind(*this, this->type, index); }

void vertex_buffer::unbind(const context& ctx, VertexBufferType type, unsigned index) const
{
	ctx.vertex_buffer_unbind(*this, type, index);
}

void vertex_buffer::bind(const context& ctx, unsigned index) const
{
	ctx.vertex_buffer_bind(*this, this->type, index);
}

void vertex_buffer::bind(const context& ctx, VertexBufferType type, unsigned index) const
{
	ctx.vertex_buffer_bind(*this, type, index);
}

bool vertex_buffer::create(const context& ctx, size_t size_in_bytes)
{
	this->size_in_bytes = size_in_bytes;
	return ctx.vertex_buffer_create(*this, 0, size_in_bytes);
}

bool vertex_buffer::is_created() const
{
	return handle != nullptr;
}

bool vertex_buffer::resize(const context& ctx, size_t size_in_bytes)
{
	if(this->size_in_bytes != size_in_bytes) {
		this->size_in_bytes = size_in_bytes;
		return ctx.vertex_buffer_resize(*this, 0, size_in_bytes);
	}
	return true;
}

bool vertex_buffer::create_or_resize(const context& ctx, size_t size_in_bytes)
{
	return !is_created() ? create(ctx, size_in_bytes) : resize(ctx, size_in_bytes);
}

bool vertex_buffer::copy(const context& ctx, size_t src_offset_in_bytes, size_t size_in_bytes, vertex_buffer& dst, size_t dst_offset_in_bytes) const
{
	return ctx.vertex_buffer_copy(*this, src_offset_in_bytes, dst, dst_offset_in_bytes, size_in_bytes);
}

void vertex_buffer::destruct(const context& ctx)
{
	if (handle) {
		ctx.vertex_buffer_destruct(*this);
		size_in_bytes = 0;
		handle = nullptr;
	}
}

} // namespace render
} // namespace cgv
