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

template <typename T> T gpu_algorithm::calculate_num_groups(T num_elements, T num_elements_per_group)
{
	assert(T(0) != num_elements_per_group);
	assert(T(0) != num_elements);
	return (num_elements + num_elements_per_group - T(1)) / num_elements_per_group;
}

template unsigned gpu_algorithm::calculate_num_groups(unsigned, unsigned);
template uvec2 gpu_algorithm::calculate_num_groups(uvec2, uvec2);
template uvec3 gpu_algorithm::calculate_num_groups(uvec3, uvec3);

}
}
