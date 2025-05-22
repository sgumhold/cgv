#include "reduce.h"

#include <cgv/math/integer.h>

namespace cgv {
namespace gpgpu {

const std::string reduce::init_argument_name = "u_init";

reduce::reduce() : reduce(256, 128) {}

reduce::reduce(uint32_t group_count, uint32_t group_size) : algorithm("reduce") {
	_num_groups = group_count;
	_group_size = group_size;
	register_kernel(_kernel, "gpgpu_reduce_group", { { "LOCAL_SIZE_X", std::to_string(group_size) } });
}

bool reduce::init(cgv::render::context& ctx, const sl::data_type& value_type) {
	return init(ctx, value_type, "");
}

bool reduce::init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& binary_operation) {
	if(!value_type.is_valid())
		return false;
	cgv::render::shader_compile_options config = get_configuration({}, { value_type });
	config.snippets.push_back({ "value_typedef", sl::get_type_alias_string("value_type", value_type) });

	if(!binary_operation.empty()) {
		config.snippets.push_back({ "operation", binary_operation });
		config.defines["USE_CUSTOM_OPERATION"] = "";
	}

	size_t available_size = static_cast<size_t>(ctx.get_device_capabilities().max_compute_shared_memory_size);
	size_t available_element_count = available_size / sl::get_aligned_size(value_type);

	if(available_element_count < _group_size)
		return false;

	if(init_kernels(ctx, config)) {
		_group_reduction_buffer.create_or_resize(ctx, value_type, _num_groups);
		return true;
	}

	return false;
}

bool reduce::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer, size_t count, const argument_bindings& arguments) {
	return dispatch(ctx, begin(buffer), begin(buffer) + count, arguments);
}

bool reduce::dispatch(cgv::render::context& ctx, device_buffer_iterator first, device_buffer_iterator last, const argument_bindings& arguments) {
	return dispatch(ctx, first, last, begin(_group_reduction_buffer), arguments);
}

bool reduce::dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output, const argument_bindings& arguments) {
	if(!is_valid_range(input_first, input_last))
		return false;

	int32_t count = cgv::gpgpu::distance(input_first, input_last);
	
	input_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	_group_reduction_buffer.bind(ctx, 1);

	_kernel.enable(ctx);
	_kernel.set_argument<uint32_t>(ctx, "u_input_begin", input_first.index());
	_kernel.set_argument<uint32_t>(ctx, "u_output_begin", 0);
	_kernel.set_argument<uint32_t>(ctx, "u_count", count);
	_kernel.set_arguments(ctx, arguments);

	dispatch_compute(_num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	input_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	_group_reduction_buffer.bind(ctx, 0);
	_group_reduction_buffer.unbind(ctx, 1);
	output.buffer().bind(ctx, 1);

	_kernel.set_argument<uint32_t>(ctx, "u_input_begin", 0);
	_kernel.set_argument<uint32_t>(ctx, "u_output_begin", output.index());
	_kernel.set_argument<uint32_t>(ctx, "u_count", _num_groups);

	dispatch_compute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	_kernel.disable(ctx);

	_group_reduction_buffer.unbind(ctx, 0);
	output.buffer().unbind(ctx, 1);

	return true;
}

} // namespace gpgpu
} // namespace cgv
