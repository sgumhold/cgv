#pragma once

#include <cgv/render/context.h>
#include <cgv/render/element_traits.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {

/** a vertex buffer is an unstructured memory block on the GPU. */
class CGV_API vertex_buffer : public vertex_buffer_base
{
	size_t size_in_bytes;
public:
	/// construct from description of component format, where the default format specifies a color buffer with alpha channel
	vertex_buffer(VertexBufferType _type = VBT_VERTICES, VertexBufferUsage _usage = VBU_STATIC_DRAW);
	/// calls the destruct method if necessary
	~vertex_buffer();
	/**
	 * Bind buffer to appropriate target.
	 *
	 * \param ctx The CGV rendering context.
	 * \param type If cgv::render::VBT_UNDEF, will use type information as given to vertex_buffer::vertex_buffer().
	 * Otherwise will bind according to the given type.
	 * \warning Pay special attention to use the same type in the matching unbind() call. Otherwise the internal type
	 * will be used again.
	 *
	 * The explicit type specification allows interpreting the underlying buffer in different ways. One example would be
	 * the manipulation of vertex data in a compute shader. Here you would bind your vertex buffer as a simple shader
	 * storage buffer before dispatching the compute call.
	 */
	void bind(context& ctx, VertexBufferType type = VBT_UNDEF) const;
	/**
	 * Unbind buffer from the appropriate target.
	 * 
	 * \param ctx The CGV rendering context.
	 * \param type If cgv::render::VBT_UNDEF, will use type information as given to vertex_buffer::vertex_buffer().
	 * Otherwise will unbind according to the given type.
	 * \warning Pay special attention to use the same type as the preceeding bind() call. Otherwise the internal type
	 * will be used again.
	 */
	void unbind(context& ctx, VertexBufferType type = VBT_UNDEF) const;
	/**
	 * Bind this buffer to the appropriate indexed buffer target.
	 *
	 * \param ctx The CGV rendering context.
	 * \param index Which index of the binding target to use.
	 *
	 * This function will use the type information given to vertex_buffer::vertex_buffer() to determine the correct
	 * binding target.
	 */
	void bind(context& ctx, unsigned index) const;
	/**
	 * Bind this buffer to the appropriate indexed buffer target.
	 *
	 * \param ctx The CGV rendering context.
	 * \param type Will bind according to the given type.
	 * \param index Which index of the binding target to use.
	 * \warning Pay special attention to use the same type in the matching unbind() call. Otherwise the internal type
	 * will be used again.
	 */
	void bind(context& ctx, VertexBufferType type, unsigned index) const;
	/// create empty vertex buffer of size \c size given in bytes
	bool create(const context& ctx, size_t size_in_bytes);
	/// create vertex buffer and copy data from CPU array \c array into buffer memory
	template <typename T>
	bool create(const context& ctx, const T& array) {
		size_in_bytes = array_descriptor_traits<T>::get_size(array);
		return ctx.vertex_buffer_create(*this, array_descriptor_traits<T>::get_address(array), size_in_bytes);
	}
	/// create vertex buffer and copy data from CPU array \c array_ptr into buffer memory
	template <typename T>
	bool create(const context& ctx, const T* array_ptr, size_t nr_elements) {
		size_in_bytes = nr_elements * sizeof(T);
		return ctx.vertex_buffer_create(*this, array_ptr, size_in_bytes);
	}
	/// check whether the vertex buffer has been created
	bool is_created() const;
	/// return size in bytes
	size_t get_size_in_bytes() const;
	/// resize vertex buffer to size \c size given in bytes clearing all data
	bool resize(const context& ctx, size_t size_in_bytes);
	/// resize vertex buffer and copy data from CPU array \c array into buffer memory
	template <typename T>
	bool resize(const context& ctx, const T& array) {
		size_in_bytes = array_descriptor_traits<T>::get_size(array);
		return ctx.vertex_buffer_resize(*this, array_descriptor_traits<T>::get_address(array), size_in_bytes);
	}
	/// resize vertex buffer and copy data from CPU array \c array_ptr into buffer memory
	template <typename T>
	bool resize(const context& ctx, const T* array_ptr, size_t nr_elements) {
		size_in_bytes = nr_elements * sizeof(T);
		return ctx.vertex_buffer_resize(*this, array_ptr, size_in_bytes);
	}
	/// replace part (starting at byte offset \c buffer_offset_in_bytes) or whole vertex buffer content from \c nr_elements of CPU array \c array_ptr
	template <typename T>
	bool replace(const context& ctx, size_t buffer_offset_in_bytes, const T* array_ptr, size_t nr_elements) {
		return ctx.vertex_buffer_replace(*this, buffer_offset_in_bytes, nr_elements* sizeof(T), array_ptr);
	}
	/// copy \c size_in_bytes number bytes from this vertex buffer starting at byte offset \c start_offset_in_bytes to vertex buffer \c dst starting at offest \c dst_offset_in_bytes
	bool copy(const context& ctx, size_t src_offset_in_bytes, size_t size_in_bytes, vertex_buffer& dst, size_t dst_offset_in_bytes) const;
	/// copy \c nr_elements array elements from vertex buffer starting at byte offset \c start_offset_in_bytes to CPU array \c array_ptr
	template <typename T>
	bool copy(const context& ctx, size_t src_offset_in_bytes, T* array_ptr, size_t nr_elements) {
		return ctx.vertex_buffer_copy_back(*this, src_offset_in_bytes, sizeof(T)*nr_elements, array_ptr);
	}
	/// destruct the render buffer
	void destruct(const context& ctx);
};

	}
}

#include <cgv/config/lib_end.h>

