#include "reduce.h"

#include <cgv/math/integer.h>

namespace cgv {
namespace gpgpu {
namespace generic {

const char* reduce::_init_argument_name = "u_init";

reduce::reduce(uint32_t group_count, GroupSize group_size) : algorithm("reduce", group_size), _num_groups(group_count) {}

bool reduce::init(cgv::render::context& ctx, const sl::data_type& value_type) {
	return init(ctx, value_type, sl::operation::plus());
}

bool reduce::init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& binary_operation) {
	size_t available_size = static_cast<size_t>(ctx.get_device_capabilities().max_compute_shared_memory_size);
	size_t available_element_count = available_size / sl::get_aligned_size(value_type);

	if(available_element_count < _group_size) {
		raise_not_enough_shared_memory_error(available_element_count, _group_size);
		return false;
	}

	algorithm_create_info info;
	info.types.push_back(value_type);
	info.typedefs.push_back({ "value_type", value_type });
	info.default_buffer_count = 2;
	info.options.define_snippet("operation", binary_operation);

	if(algorithm::init(ctx, info, { { &_kernel, "gpgpu_reduce_group" } })) {
		_group_reduction_buffer.create_or_resize(ctx, value_type, _num_groups);
		return true;
	}

	return false;
}

void reduce::destruct(const cgv::render::context& ctx) {
	_kernel.destruct(ctx);
	_group_reduction_buffer.destruct(ctx);
	algorithm::destruct(ctx);
}

bool reduce::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, const argument_bindings& arguments) {
	return dispatch(ctx, begin(buffer), begin(buffer) + count, arguments);
}

bool reduce::dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, const argument_bindings& arguments) {
	return dispatch(ctx, first, last, begin(_group_reduction_buffer), arguments);
}

bool reduce::dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output, const argument_bindings& arguments) {
	if(!is_valid_range(input_first, input_last)) {
		raise_error(errc::invalid_range);
		return false;
	}

	uint32_t count = static_cast<uint32_t>(cgv::gpgpu::distance(input_first, input_last));
	uint32_t num_groups = std::min(_num_groups, cgv::math::div_round_up(count, _group_size));

	input_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	// If we just need one group we can directly write the result to the output buffer and don't need to reduce over the partial group results.
	if(num_groups == 1)
		output.buffer().bind(ctx, 1);
	else
		_group_reduction_buffer.bind(ctx, 1);

	_kernel.enable(ctx);
	_kernel.set_argument<uint32_t>(ctx, "u_input_begin", input_first.index());
	_kernel.set_argument<uint32_t>(ctx, "u_output_begin", 0);
	_kernel.set_argument<uint32_t>(ctx, "u_count", count);
	_kernel.set_arguments(ctx, arguments);

	dispatch_compute(num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	input_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	if(num_groups == 1)
		output.buffer().unbind(ctx, 1);
	else
		_group_reduction_buffer.unbind(ctx, 1);

	// If we need more than one group the partial results are contained in _group_reduction_buffer and we need to run a second reduction pass to reduce over those.
	if(num_groups > 1) {
		_group_reduction_buffer.bind(ctx, 0);
		output.buffer().bind(ctx, 1);

		_kernel.set_argument<uint32_t>(ctx, "u_input_begin", 0);
		_kernel.set_argument<uint32_t>(ctx, "u_output_begin", output.index());
		_kernel.set_argument<uint32_t>(ctx, "u_count", num_groups);

		dispatch_compute(1, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	_kernel.disable(ctx);

	_group_reduction_buffer.unbind(ctx, 0);
	output.buffer().unbind(ctx, 1);

	return true;
}

} // namespace generic
} // namespace gpgpu
} // namespace cgv
