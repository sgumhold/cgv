#include "gpu_algorithm.h"

namespace cgv {
namespace gpgpu {

void gpu_algorithm::ensure_buffer(const cgv::render::context& ctx, cgv::render::vertex_buffer& buffer, size_t size, cgv::render::VertexBufferType type, cgv::render::VertexBufferUsage usage) {

	if(buffer.is_created() && buffer.type == type && buffer.usage == usage) {
		buffer.resize(ctx, size);
	} else {
		buffer.destruct(ctx);
		buffer = cgv::render::vertex_buffer(type, usage);
		buffer.create(ctx, size);
	}
}

void gpu_algorithm::delete_buffer(const cgv::render::context& ctx, cgv::render::vertex_buffer& buffer) {

	buffer.destruct(ctx);
}

}
}
