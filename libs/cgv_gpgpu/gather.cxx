#include "gather.h"

#include <cgv/math/integer.h>

namespace cgv {
namespace gpgpu {

gather::gather(uint32_t group_size) : algorithm("gather", group_size) {}

bool gather::init(cgv::render::context& ctx, const sl::data_type& value_type) {
	if(!value_type.is_valid())
		return false;

	_uniform_buffer.create(ctx);

	algorithm_create_info info;
	info.types.push_back(value_type);
	info.typedefs.push_back({ "value_type", value_type });
	info.default_buffer_count = 3;
	return algorithm::init(ctx, info, { { &_kernel, "gpgpu_gather" } });
}

void gather::destruct(const cgv::render::context& ctx) {
	_kernel.destruct(ctx);
	_uniform_buffer.destruct(ctx);
	algorithm::destruct(ctx);
}

bool gather::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& map, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count) {
	return dispatch(ctx, begin(map), begin(map) + count, begin(input_buffer), begin(output_buffer));
}

bool gather::dispatch(cgv::render::context& ctx, device_buffer_iterator map_first, device_buffer_iterator map_last, device_buffer_iterator input_first, device_buffer_iterator output_first) {
	if(!is_valid_range(map_first, map_last))
		return false;

	if(compatible(input_first, output_first))
		return false;

	input_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
	map_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 2);

	uniform_data uniforms;
	uniforms.map_begin = static_cast<uint32_t>(map_first.index());
	uniforms.map_end = static_cast<uint32_t>(map_last.index());
	uniforms.input_begin = static_cast<uint32_t>(input_first.index());
	uniforms.output_begin = static_cast<uint32_t>(output_first.index());
	_uniform_buffer.replace(ctx, uniforms);
	_uniform_buffer.bind(ctx, 0);

	_kernel.enable(ctx);
	
	uint32_t num_groups = cgv::math::div_round_up(static_cast<uint32_t>(cgv::gpgpu::distance(map_first, map_last)), _group_size);
	dispatch_compute(num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	_kernel.disable(ctx);

	_uniform_buffer.unbind(ctx, 0);
	input_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
	map_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 2);

	return true;
}

} // namespace gpgpu
} // namespace cgv
