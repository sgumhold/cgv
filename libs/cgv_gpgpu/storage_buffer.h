#pragma once

#include <cgv/math/integer.h>
#include <cgv/render/context.h>
#include <cgv/render/vertex_buffer.h>

#include "sl.h"

namespace cgv {
namespace gpgpu {

/// A small helper class for handling device buffers of arbitrary storage.
class storage_buffer : public cgv::render::vertex_buffer {
public:
	storage_buffer(cgv::render::VertexBufferUsage usage = cgv::render::VertexBufferUsage::VBU_STREAM_COPY) : cgv::render::vertex_buffer(cgv::render::VertexBufferType::VBT_STORAGE, usage) {}

	template<typename T>
	bool create_or_resize(const cgv::render::context& ctx, size_t element_count) {
		return cgv::render::vertex_buffer::create_or_resize(ctx, element_count * sizeof(T));
	}

	template <typename T, typename = std::enable_if_t<std::is_class<T>::value, bool>>
	bool create_or_resize(const cgv::render::context& ctx, const T& array) {
		return cgv::render::vertex_buffer::create_or_resize(ctx, array);
	}

	bool create_or_resize(const cgv::render::context& ctx, sl::data_type element_type, size_t element_count) {
		return cgv::render::vertex_buffer::create_or_resize(ctx, element_count * sl::get_aligned_size(element_type));
	}
};

} // namespace gpgpu
} // namespace cgv
