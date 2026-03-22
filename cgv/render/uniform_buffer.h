#pragma once

#include "context.h"
#include "vertex_buffer.h"

namespace cgv {
namespace render {

/// @brief Helper class template for handling vertex buffers of type uniform buffer object
/// that explicitly states the type of data stored.
template<class T>
class uniform_buffer : public vertex_buffer {
public:
	/// @brief Construct with the given usage.
	/// 
	/// @param usage The default usage of this buffer.
	uniform_buffer(VertexBufferUsage usage = VertexBufferUsage::VBU_STREAM_COPY) : vertex_buffer(VertexBufferType::VBT_UNIFORM, usage) {}

	/// @brief Create an empty buffer of size sizeof(T).
	/// 
	/// @param ctx The CGV rendering context.
	/// @return True if success, false otherwise.
	bool create(const context& ctx) {
		return vertex_buffer::create(ctx, sizeof(T));
	}

	/// @brief Create an empty buffer of size array_size * sizeof(T).
	/// 
	/// @param ctx The CGV rendering context.
	/// @param array_size The number of array elements of T to allocate space for.
	/// @return True if success, false otherwise.
	bool create(const context& ctx, size_t array_size) {
		return vertex_buffer::create(ctx, sizeof(T) * array_size);
	}

	/// @brief Resize to array_size * sizeof(T).
	/// 
	/// @param ctx The CGV rendering context.
	/// @param array_size The number of array elements of T to allocate space for.
	/// @return True if success, false otherwise.
	bool resize(const context& ctx, size_t array_size) {
		return vertex_buffer::resize(ctx, sizeof(T) * array_size);
	}

	/// @brief Resize to sizeof(T) and set the given data.
	/// 
	/// @param ctx The CGV rendering context.
	/// @param data The new buffer data.
	/// @return True if success, false otherwise.
	bool resize(const context& ctx, const T& data) {
		return vertex_buffer::resize(ctx, reinterpret_cast<const uint8_t*>(&data), sizeof(T));
	}

	/// @brief Resize to array.size() * sizeof(T) and set the given array as data.
	/// 
	/// @param ctx The CGV rendering context.
	/// @param array The new buffer data.
	/// @return True if success, false otherwise.
	bool resize(const context& ctx, const std::vector<T>& array) {
		return vertex_buffer::resize(ctx, reinterpret_cast<const uint8_t*>(array.data()), sizeof(T) * array.size());
	}

	/// @brief Replace the content with the given data.
	/// The buffer should have a size equivalent to that of a single element of T.
	/// Otherwise, only the first sizeof(T) bytes of the buffer will be overwritten, with the
	/// remaining bytes left untouched.
	/// 
	/// @param ctx The CGV rendering context.
	/// @param data The new buffer data.
	/// @return True if success, false otherwise.
	bool replace(const context& ctx, const T& data) {
		return vertex_buffer::replace(ctx, 0, reinterpret_cast<const uint8_t*>(&data), sizeof(T));
	}

	/// @brief Replace the content with the given array.
	/// The buffer must have a size in bytes of at least array.size() * sizeof(T).
	/// All buffer data beyond the array size is left untouched.
	/// 
	/// @param ctx The CGV rendering context.
	/// @param array The new buffer data.
	/// @return True if success, false otherwise.
	bool replace(const context& ctx, const std::vector<T>& array) {
		return vertex_buffer::replace(ctx, 0, reinterpret_cast<const uint8_t*>(array.data()), sizeof(T) * array.size());
	}

	/// @brief Resize to sizeof(T) if necessary and set the given data.
	/// In contrast to resize(), will not resize the buffer if it already has the exact size needed.
	/// 
	/// @param ctx The CGV rendering context.
	/// @param data The new buffer data.
	/// @return True if success, false otherwise.
	bool resize_or_replace(const context& ctx, const T& data) {
		return size_in_bytes != sizeof(T) ? resize(ctx, data) : replace(ctx, data);
	}

	/// @brief Resize to array.size() * sizeof(T) if necessary and set the given array as data.
	/// In contrast to resize(), will not resize the buffer if it already has the exact size needed.
	/// 
	/// @param ctx The CGV rendering context.
	/// @param data The new buffer data.
	/// @return True if success, false otherwise.
	bool resize_or_replace(const context& ctx, std::vector<T>& array) {
		return size_in_bytes != sizeof(T) ? resize(ctx, array) : replace(ctx, array);
	}

	/// @brief Deleted to avoid resizing this buffer to any arbitrary size.
	bool create_or_resize() = delete;
};

} // namespace render
} // namespace cgv
