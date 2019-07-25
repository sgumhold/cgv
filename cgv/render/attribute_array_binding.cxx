#include <cgv/base/base.h>
#include "attribute_array_binding.h"
#include "shader_program.h"
#include <cgv/render/vertex_buffer.h>

namespace cgv {
	namespace render {

bool attribute_array_binding::set_global_attribute_array(const context& ctx, int loc, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes) 
{
	return ctx.set_attribute_array_void(0, loc, element_type, &vbo, reinterpret_cast<const void*>(offset_in_bytes), nr_elements, stride_in_bytes);
}

/// set the global elment array to the given vertex buffer object which must me of type VBT_INDICES
bool attribute_array_binding::set_global_element_array(const context& ctx, const vertex_buffer& vbe)
{
	return ctx.set_element_array(0, &vbe);
}

/// point array of vertex attribute at location \c loc to elements of given type in vertex buffer object at given offset spaced with given stride; in case of success also enable vertex attribute array
bool attribute_array_binding::set_attribute_array(const context& ctx, int loc, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes) 
{
	return ctx.set_attribute_array_void(this, loc, element_type, &vbo, reinterpret_cast<const void*>(offset_in_bytes), nr_elements, stride_in_bytes);
}

int attribute_array_binding::get_attribute_location(const context& ctx, const shader_program& prog, const std::string& attr_name) const
{
	return prog.get_attribute_location(ctx, attr_name);
}

/// conveniance function that determines attribute location in program by name and then uses set_attribute_array to point array of vertex attribute at location \c loc to elements of given type in vertex buffer object at given offset spaced with given stride
bool attribute_array_binding::bind_attribute_array(const context& ctx, const shader_program& prog, const std::string& attribute_name, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes)
{
	int loc = prog.get_attribute_location(ctx, attribute_name);
	if (loc == -1)
		return false;
	return set_attribute_array(ctx, loc, element_type, vbo, offset_in_bytes, nr_elements, stride_in_bytes);
}

/// set the elment array to the given vertex buffer object which must me of type VBT_INDICES
bool attribute_array_binding::set_element_array(const context& ctx, const vertex_buffer& vbe)
{
	return ctx.set_element_array(this, &vbe);
}

/// check whether an attribute array is enabled
bool attribute_array_binding::is_global_array_enabled(const context& ctx, int loc) 
{
	return ctx.is_attribute_array_enabled(0, loc);
}

/// enable attribute array of given location
bool attribute_array_binding::enable_global_array(const context& ctx, int loc) 
{
	return ctx.enable_attribute_array(0, loc, true);
}
/// disable attribute array of given location
bool attribute_array_binding::disable_global_array(const context& ctx, int loc) 
{
	return ctx.enable_attribute_array(0, loc, false);
}

/// point array of vertex attribute at location \c loc to vertex buffer array \c array stored in CPU memory; in case of success also enable vertex attribute array
bool attribute_array_binding::set_global_attribute_array(const context& ctx, int loc, const vertex_buffer& vbo, type_descriptor td, size_t size, size_t offset, unsigned stride) 
{
	const void* ptr = 0;
	reinterpret_cast<size_t&>(ptr) = offset;
	return ctx.set_attribute_array_void(0, loc, td, &vbo, ptr, size, stride);
}

/** create empty shader program and set the option whether errors during
shader code attachment should be printed to std::cerr */
attribute_array_binding::attribute_array_binding()
{
}
/// call destruct method
attribute_array_binding::~attribute_array_binding()
{
	if (ctx_ptr) {
		if (ctx_ptr->make_current()) {
			destruct(*ctx_ptr);
			ctx_ptr = 0;
		}
	}
}

/// create the attribute array binding
bool attribute_array_binding::create(const context& ctx)
{
	return ctx.attribute_array_binding_create(*this);
}

/// destruct attribute array
void attribute_array_binding::destruct(const context& ctx)
{
	if (handle)
		ctx.attribute_array_binding_destruct(*this);
}

/// enable the attribute array binding
bool attribute_array_binding::enable(context& ctx)
{
	if (!is_created()) {
		ctx.error("attribute_array_binding::enable() attribute array binding must be created before enable", this);
		return false;
	}
	return ctx.attribute_array_binding_enable(*this);
}

/// disable the attribute array binding
bool attribute_array_binding::disable(context& ctx)
{
	return ctx.attribute_array_binding_disable(*this);
}

///
bool attribute_array_binding::enable_array(const context& ctx, int loc)
{
	return ctx.enable_attribute_array(this, loc, true);
}
/// 
bool attribute_array_binding::is_array_enabled(const context& ctx, int loc) const
{
	if (loc < 0)
		return false;
	return ctx.is_attribute_array_enabled(this, loc);
}
///
bool attribute_array_binding::disable_array(const context& ctx, int loc)
{
	return ctx.enable_attribute_array(this, loc, false);
}

	}
}