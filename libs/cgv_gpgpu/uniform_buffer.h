#pragma once

#include <cgv/render/context.h>
#include <cgv/render/vertex_buffer.h>

#include "sl.h"

namespace cgv {
namespace gpgpu {

/// A small helper class for handling device buffers of type uniform buffer object.
template<class T>
class uniform_buffer : public cgv::render::vertex_buffer {
public:
	uniform_buffer(cgv::render::VertexBufferUsage usage = cgv::render::VertexBufferUsage::VBU_STREAM_COPY) : cgv::render::vertex_buffer(cgv::render::VertexBufferType::VBT_UNIFORM, usage) {}

	bool create(cgv::render::context& ctx) {
		return vertex_buffer::create(ctx, sizeof(T));
	}

	bool replace(cgv::render::context& ctx, const T& data) {
		return vertex_buffer::replace(ctx, 0, reinterpret_cast<const uint8_t*>(&data), sizeof(T));
	}

	bool create_or_resize() = delete;
	bool resize() = delete;
};

} // namespace gpgpu
} // namespace cgv
