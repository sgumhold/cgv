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

	bool create_or_resize(const cgv::render::context& ctx, sl::data_type element_type, size_t element_count) {
		size_t aligned_element_size = cgv::math::next_multiple_k_greater_than_n(element_type.alignment_in_bytes(), element_type.size_in_bytes());
		return cgv::render::vertex_buffer::create_or_resize(ctx, element_count * aligned_element_size);
	}
};

} // namespace gpgpu
} // namespace cgv
