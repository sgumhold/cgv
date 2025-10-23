#include "copy.h"

#include <cgv/math/integer.h>

namespace cgv {
namespace gpgpu {

copy::copy(uint32_t group_size) : algorithm("copy", group_size) {}

bool copy::init(cgv::render::context& ctx, const sl::data_type& value_type) {
	if(!value_type.is_valid())
		return false;

	algorithm_create_info info;
	info.types.push_back(value_type);
	info.typedefs.push_back({ "value_type", value_type });
	info.default_buffer_count = 2;

	return algorithm::init(ctx, info, { { &_kernel, "gpgpu_copy" } });
}

void copy::destruct(const cgv::render::context& ctx) {
	_kernel.destruct(ctx);
	algorithm::destruct(ctx);
}

bool copy::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count) {
	return dispatch(ctx, begin(input_buffer), begin(input_buffer) + count, begin(output_buffer));
}

bool copy::dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first) {
	if(!is_valid_range(input_first, input_last))
		return false;

	if(compatible(input_first, output_first))
		return false;
	
	input_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
	
	_kernel.enable(ctx);
	_kernel.set_argument<uint32_t>(ctx, "u_input_begin", input_first.index());
	_kernel.set_argument<uint32_t>(ctx, "u_input_end", input_last.index());
	_kernel.set_argument<uint32_t>(ctx, "u_output_begin", output_first.index());

	uint32_t num_groups = cgv::math::div_round_up(static_cast<uint32_t>(cgv::gpgpu::distance(input_first, input_last)), _group_size);
	dispatch_compute(num_groups, 1, 1);;
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	
	_kernel.disable(ctx);

	input_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);

	return true;
}

} // namespace gpgpu
} // namespace cgv
