#include "scan.h"

#include <cgv/math/integer.h>

namespace cgv {
namespace gpgpu {
namespace detail {

const std::string scan::init_argument_name = "u_init";

scan::scan(const std::string& name, bool inclusive, uint32_t group_size) : algorithm(name, group_size), _inclusive(inclusive) {}

bool scan::init(cgv::render::context& ctx, const sl::data_type& value_type) {
	if(!value_type.is_valid())
		return false;

	size_t available_size = static_cast<size_t>(ctx.get_device_capabilities().max_compute_shared_memory_size);
	size_t available_element_count = available_size / sl::get_aligned_size(value_type);

	if(available_element_count < static_cast<size_t>(2 * _group_size))
		return false;

	_value_type = value_type;
	
	algorithm_create_info info;
	info.types.push_back(value_type);
	info.typedefs.push_back({ "value_type", value_type });
	info.default_buffer_count = 3;
	info.options.define_macro_if_true(_inclusive, "INCLUSIVE_SCAN");

	cgv::render::shader_compile_options global_scan_options;
	global_scan_options.define_macro("LOCAL_SIZE_X", ctx.get_device_capabilities().max_compute_work_group_size.x());
	// The global scan can be performed in a raking fashion using shared memory to sequentially scan parts of the input
	// array and propagate the partial sums. This can be enabled by uncommenting the following line. However, in practice
	// this has shown worse performance than the naive approach. One must also ensure that enough shared memory is available
	// for the used work group size and value_type.
	//global_scan_options.define_macro("USE_SHARED_MEMORY");
	
	std::vector<compute_kernel_info> kernels = {
		{ &_scan_local_kernel, "gpgpu_scan_scan_local" },
		{ &_scan_global_kernel, "gpgpu_scan_scan_global", global_scan_options },
		{ &_propagate_kernel, "gpgpu_scan_propagate" }
	};

	_part_size = 2 * _group_size;
	_uniform_buffer.create(ctx);

	return algorithm::init(ctx, info, kernels);
}

void scan::destruct(const cgv::render::context& ctx) {
	_uniform_buffer.destruct(ctx);
	_scan_local_kernel.destruct(ctx);
	_scan_global_kernel.destruct(ctx);
	_propagate_kernel.destruct(ctx);
	_group_sums_buffer.destruct(ctx);
	algorithm::destruct(ctx);
}

void scan::resize(cgv::render::context& ctx, uint32_t size) {
	_num_groups = cgv::math::div_round_up(size, _part_size);
	_uniforms.num_group_sums = cgv::math::next_power_of_two(_num_groups);

	_group_sums_buffer.create_or_resize(ctx, _value_type, _uniforms.num_group_sums);
}

bool scan::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count, const argument_bindings& arguments) {
	return dispatch(ctx, begin(input_buffer), begin(input_buffer) + count, begin(output_buffer), arguments);
}

bool scan::dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, const argument_bindings& arguments) {
	if(!is_valid_range(input_first, input_last))
		return false;

	if(compatible(input_first, output_first))
		return false;

	uint32_t count = static_cast<uint32_t>(cgv::gpgpu::distance(input_first, input_last));
	uint32_t max_count = static_cast<uint32_t>(ctx.get_device_capabilities().max_compute_work_group_count.x()) * _part_size;
	if(count > max_count)
		return false;

	if(count != _last_size) {
		resize(ctx, count);
		_last_size = count;
	}

	_uniforms.input_begin = static_cast<uint32_t>(input_first.index());
	_uniforms.input_end = static_cast<uint32_t>(input_last.index());
	_uniforms.output_begin = static_cast<uint32_t>(output_first.index());
	_uniform_buffer.replace(ctx, _uniforms);
	_uniform_buffer.bind(ctx, 0);

	input_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
	_group_sums_buffer.bind(ctx, 2);
	
	_scan_local_kernel.enable(ctx);
	_scan_local_kernel.set_arguments(ctx, arguments);
	dispatch_compute(_num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	_scan_local_kernel.disable(ctx);

	_scan_global_kernel.enable(ctx);
	dispatch_compute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	_scan_global_kernel.disable(ctx);

	_propagate_kernel.enable(ctx);
	dispatch_compute(_num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	_propagate_kernel.disable(ctx);
	
	optional_memory_barrier(output_first.buffer().type);

	input_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
	_group_sums_buffer.unbind(ctx, 2);

	_uniform_buffer.unbind(ctx, 0);

	return true;
}

} // namespace detail
} // namespace gpgpu
} // namespace cgv
