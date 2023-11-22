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
	 * \param index The index to unbind from, on an index binding target.
	 * \warning Pay special attention to use the same type as the preceeding bind() call. Otherwise the internal type
	 * will be used again.
	 */
	void unbind(context& ctx, VertexBufferType type = VBT_UNDEF) const;
	/// @brief Unbind this buffer from the appropriate indexed binding target
	/// @param ctx The CGV rendering context.
	/// @param index Which slot of the target to unbind from.
	void unbind(context& ctx, unsigned index) const;
	/// @brief Unbind from the choosen binding target.
	/// @param ctx The CGV rendering context.
	/// @param type If cgv::render::VBT_UNDEF, will use type information as given to vertex_buffer::vertex_buffer().
	/// Otherwise will unbind according to the given type.
	/// @param index Which slot of the target to unbind from.
	/// @warning Pay special attention to use the same type as the preceeding bind() call. Otherwise the internal type
	/// will be used again.
	void unbind(context& ctx, VertexBufferType type, unsigned index) const;
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
	bool create(const context& ctx, const T& array)
	{
		size_in_bytes = array_descriptor_traits<T>::get_size(array);
		return ctx.vertex_buffer_create(*this, array_descriptor_traits<T>::get_address(array), size_in_bytes);
	}
	/// create vertex buffer and copy data from CPU array \c array_ptr into buffer memory
	template <typename T>
	bool create(const context& ctx, const T* array_ptr, size_t nr_elements) {
		size_in_bytes = nr_elements * sizeof(T);
		return ctx.vertex_buffer_create(*this, array_ptr, size_in_bytes);
	}
	/**
	 * Check whether the vertex buffer has been created.
	 * 
	 * \return true if the buffer has a handle from the graphics context, false otherwise.
	 */
	bool is_created() const override;
	/**
	 * Retrieves the current size of the buffer in bytes.
	 * 
	 * \return buffer size of the last allocation command in bytes.
	 */
	size_t get_size_in_bytes() const;
	/// resize vertex buffer to size \c size given in bytes clearing all data
	bool resize(const context& ctx, size_t size_in_bytes);
	/// resize vertex buffer and copy data from CPU array \c array into buffer memory
	template <typename T, typename = std::enable_if_t<std::is_class<T>::value, bool>>
	bool resize(const context& ctx, const T& array)
	{
		size_in_bytes = array_descriptor_traits<T>::get_size(array);
		return ctx.vertex_buffer_resize(*this, array_descriptor_traits<T>::get_address(array), size_in_bytes);
	}
	/// resize vertex buffer and copy data from CPU array \c array_ptr into buffer memory
	template <typename T>
	bool resize(const context& ctx, const T* array_ptr, size_t nr_elements) {
		size_in_bytes = nr_elements * sizeof(T);
		return ctx.vertex_buffer_resize(*this, array_ptr, size_in_bytes);
	}
	/**
	 * Convenience wrapper to either create() or resize() the buffer.
	 * 
	 * \param ctx The CGV rendering context.
	 * \param size_in_bytes The desired size in bytes of the buffer.
	 * \return False if there was a rendering API error, true otherwise.
	 */
	bool create_or_resize(const context& ctx, size_t size_in_bytes);
	/**
	 * Convenience wrapper to either create() or resize() the buffer.
	 * 
	 * \tparam T A generic array type.
	 * \param ctx The CGV rendering context.
	 * \param array The array which will be copied into the buffer.
	 * \return False if there was a rendering API error, true otherwise.
	 */
	template <typename T, typename = std::enable_if_t<std::is_class<T>::value, bool>>
	bool create_or_resize(const context& ctx, const T& array)
	{
		return !is_created() ? create(ctx, array) : resize(ctx, array);
	}
	/**
	 * Convenience wrapper to either create() or resize() the buffer.
	 *
	 * \tparam T the fundamental data type of the CPU array.
	 * \param ctx The CGV rendering context.
	 * \param array_ptr The array which will be copied into the buffer.
	 * \param nr_elements The count of elements in the array.
	 * \return False if there was a rendering API error, true otherwise.
	 */
	template <typename T> bool create_or_resize(const context& ctx, const T* array_ptr, size_t nr_elements)
	{
		return !is_created() ? create(ctx, array_ptr, nr_elements) : resize(ctx, array_ptr, nr_elements);
	}
	/// replace part (starting at byte offset \c buffer_offset_in_bytes) or whole vertex buffer content from \c nr_elements of CPU array \c array_ptr
	template <typename T>
	bool replace(const context& ctx, size_t buffer_offset_in_bytes, const T* array_ptr, size_t nr_elements) {
		return ctx.vertex_buffer_replace(*this, buffer_offset_in_bytes, nr_elements* sizeof(T), array_ptr);
	}
	/**
	 * Copy bytes between different vertex_buffer instances.
	 * 
	 * \param ctx The CGV rendering context.
	 * \param src_offset_in_bytes Offset in bytes from the beginning of the source buffer.
	 * \param size_in_bytes The amount of bytes, which will be copied.
	 * \param dst The buffer in which to copy the bytes into.
	 * \param dst_offset_in_bytes Offset in bytes from the beginning of the destination buffer.
	 * \return False if the buffers have no valid handles or if there was a rendering API error, true otherwise.
	 */
	bool copy(const context& ctx, size_t src_offset_in_bytes, size_t size_in_bytes, vertex_buffer& dst, size_t dst_offset_in_bytes) const;
	/**
	 * Copy elements from the buffer into the CPU array.
	 * 
	 * \tparam T the fundamental data type of the CPU array.
	 * \param ctx The CGV rendering context.
	 * \param src_offset_in_bytes Offset in bytes from the beginning of the source buffer.
	 * \param array_ptr The destination array in which to copy the elements.
	 * \param nr_elements How many of the elements to copy into the destination buffer.
	 * \return False if the vertex_buffer has no valid handle or if there was a rendering API error, true otherwise.
	 */
	template <typename T>
	bool copy(const context& ctx, size_t src_offset_in_bytes, T* array_ptr, size_t nr_elements) {
		return ctx.vertex_buffer_copy_back(*this, src_offset_in_bytes, sizeof(T)*nr_elements, array_ptr);
	}
	/**
	 * Copy elements from the buffer into the CPU array.
	 * 
	 * \tparam T A generic array type.
	 * \param ctx The CGV rendering context.
	 * \param array The destination array in which to copy the data.
	 * \param src_offset_in_bytes Offset in bytes from the beginning of the source buffer.
	 * \return False if the vertex_buffer has no valid handle or if there was a rendering API error, true otherwise.
	 * 
	 * This function will implicity infer how many bytes to copy from the dimensions of the generic array. Therefore you
	 * need to make sure, that your std::vector or cgv::math::vec is appropriately sized beforehand!
	 * Otherwise the underlying copy command might cross the boundaries of the GPU buffer and or read unrelated data.
	 */
	template <typename T, typename = std::enable_if_t<std::is_class<T>::value, bool>>
	bool copy(const context& ctx, T& array, size_t src_offset_in_bytes = 0)
	{
		const auto size_in_bytes = array_descriptor_traits<T>::get_size(array);
		return ctx.vertex_buffer_copy_back(*this, src_offset_in_bytes, size_in_bytes,
										   array_descriptor_traits<T>::get_address(array));
	}
	/// destruct the render buffer
	void destruct(const context& ctx);
};

	}
}

#include <cgv/config/lib_end.h>

