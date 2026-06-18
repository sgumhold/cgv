#include "scatter.h"

#include <cgv/math/integer.h>

namespace cgv {
namespace gpgpu {

scatter::scatter(uint32_t group_size) : algorithm("scatter", group_size) {}

bool scatter::init(cgv::render::context& ctx, const sl::data_type& value_type) {
	if(!value_type.is_valid())
		return false;

	_uniform_buffer.create(ctx);

	algorithm_create_info info;
	info.types.push_back(value_type);
	info.typedefs.push_back({ "value_type", value_type });
	info.default_buffer_count = 3;
	return algorithm::init(ctx, info, { { &_kernel, "gpgpu_scatter" } });
}

void scatter::destruct(const cgv::render::context& ctx) {
	_kernel.destruct(ctx);
	_uniform_buffer.destruct(ctx);
	algorithm::destruct(ctx);
}

bool scatter::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& map, const cgv::render::vertex_buffer& output_buffer, size_t count) {
	return dispatch(ctx, begin(input_buffer), begin(input_buffer) + count, begin(map), begin(output_buffer));
}

bool scatter::dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator map_first, device_buffer_iterator output_first) {
	if(!is_valid_range(input_first, input_last))
		return false;

	if(compatible(input_first, output_first))
		return false;

	input_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
	map_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 2);

	uniform_data uniforms;
	uniforms.input_begin = static_cast<uint32_t>(input_first.index());
	uniforms.input_end = static_cast<uint32_t>(input_last.index());
	uniforms.output_begin = static_cast<uint32_t>(output_first.index());
	uniforms.map_begin = static_cast<uint32_t>(map_first.index());
	_uniform_buffer.replace(ctx, uniforms);
	_uniform_buffer.bind(ctx, 0);

	_kernel.enable(ctx);
	
	uint32_t num_groups = cgv::math::div_round_up(static_cast<uint32_t>(cgv::gpgpu::distance(input_first, input_last)), _group_size);
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
