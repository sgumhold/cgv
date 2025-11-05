#include "copy_if.h"

#include <cgv/math/integer.h>

namespace cgv {
namespace gpgpu {
namespace detail {

write_if::write_if(const std::string& name, bool write_indices) : algorithm(name), _write_indices(write_indices) {}

bool write_if::init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& unary_predicate) {
	return init(ctx, value_type, {}, unary_predicate);
}

bool write_if::init(cgv::render::context& ctx, const sl::data_type& value_type, const argument_definitions& arguments, const std::string& unary_predicate) {
	if(!value_type.is_valid())
		return false;

	algorithm_create_info info;
	info.arguments = &arguments;
	info.types.push_back(value_type);
	info.typedefs.push_back({ "value_type", value_type });
	info.default_buffer_count = 6;
	info.options.define_macro_if_true(_write_indices, "WRITE_INDICES");
	info.options.define_snippet("predicate", unary_predicate);

	std::vector<compute_kernel_info> kernels = {
		{ &_vote_kernel, "gpgpu_copy_if_vote" },
		{ &_scan_local_kernel, "gpgpu_copy_if_scan_local" },
		{ &_scan_global_kernel, "gpgpu_copy_if_scan_global" },
		{ &_scatter_kernel, "gpgpu_copy_if_scatter" }
	};

	_uniform_buffer.create(ctx);
	_last_sum_buffer.create_or_resize<uint32_t>(ctx, 1);

	return algorithm::init(ctx, info, kernels);
}

void write_if::destruct(const cgv::render::context& ctx) {
	_uniform_buffer.destruct(ctx);
	_vote_kernel.destruct(ctx);
	_scan_local_kernel.destruct(ctx);
	_scan_global_kernel.destruct(ctx);
	_scatter_kernel.destruct(ctx);
	_votes_buffer.destruct(ctx);
	_prefix_sums_buffer.destruct(ctx);
	_block_sums_buffer.destruct(ctx);
	_last_sum_buffer.destruct(ctx);
	algorithm::destruct(ctx);
}

void write_if::resize(cgv::render::context& ctx, uint32_t size) {
	uint32_t num_values_padded = cgv::math::next_multiple_k_greater_than_n(_block_size, size);
	uint32_t num_vote_ballots = cgv::math::div_round_up(num_values_padded, 32u);

	_num_groups = cgv::math::div_round_up(num_values_padded, _block_size);
	_num_block_sums = cgv::math::next_power_of_two(_num_groups);

	_votes_buffer.create_or_resize<uint32_t>(ctx, num_vote_ballots);
	_prefix_sums_buffer.create_or_resize<uint32_t>(ctx, num_values_padded);
	_block_sums_buffer.create_or_resize<uint32_t>(ctx, _num_block_sums);

	uint32_t block_sum_offset_shift = static_cast<uint32_t>(std::log2(_block_size));
	_last_block_sum_idx = (num_values_padded >> block_sum_offset_shift) - 1;
}

bool write_if::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count, const argument_bindings& arguments) {
	return dispatch(ctx, begin(input_buffer), begin(input_buffer) + count, begin(output_buffer), arguments);
}

bool write_if::dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, const argument_bindings& arguments) {
	return dispatch(ctx, input_first, input_last, output_first, begin(_last_sum_buffer), arguments);
}

bool write_if::dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, device_buffer_iterator output_count, const argument_bindings& arguments) {
	if(!is_valid_range(input_first, input_last))
		return false;

	if(compatible(input_first, output_first))
		return false;

	uint32_t count = static_cast<uint32_t>(cgv::gpgpu::distance(input_first, input_last));
	uint32_t max_count = static_cast<uint32_t>(ctx.get_device_capabilities().max_compute_work_group_count.x()) * _block_size;
	if(count > max_count)
		return false;

	if(count != _last_size) {
		resize(ctx, count);
		_last_size = count;
	}

	uniform_data uniforms;
	uniforms.input_begin = static_cast<uint32_t>(input_first.index());
	uniforms.input_end = static_cast<uint32_t>(input_last.index());
	uniforms.output_begin = static_cast<uint32_t>(output_first.index());
	uniforms.output_count_begin = static_cast<uint32_t>(output_count.index());
	uniforms.num_block_sums = _num_block_sums;
	uniforms.last_block_sum_idx = _last_block_sum_idx;
	_uniform_buffer.replace(ctx, uniforms);
	_uniform_buffer.bind(ctx, 0);

	input_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
	_votes_buffer.bind(ctx, 2);

	_vote_kernel.enable(ctx);
	_vote_kernel.set_arguments(ctx, arguments);
	bind_buffer_like_arguments(ctx, arguments);

	dispatch_compute(_num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	unbind_buffer_like_arguments(ctx, arguments);
	_vote_kernel.disable(ctx);

	_prefix_sums_buffer.bind(ctx, 3);
	_block_sums_buffer.bind(ctx, 4);
	output_count.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 5);

	_scan_local_kernel.enable(ctx);
	dispatch_compute(_num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	_scan_local_kernel.disable(ctx);

	_scan_global_kernel.enable(ctx);
	dispatch_compute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	_scan_global_kernel.disable(ctx);

	_scatter_kernel.enable(ctx);
	dispatch_compute(_num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	_scatter_kernel.disable(ctx);

	optional_memory_barrier(output_count.buffer().type);

	input_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
	_votes_buffer.unbind(ctx, 2);
	_prefix_sums_buffer.unbind(ctx, 3);
	_block_sums_buffer.unbind(ctx, 4);
	output_count.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 5);

	_uniform_buffer.unbind(ctx, 0);

	return true;
}

bool write_if::read_count(cgv::render::context& ctx, size_t& out) {
	uint32_t count = 0;
	if(_last_sum_buffer.copy(ctx, 0, &count, 1)) {
		out = count;
		return true;
	}
	return false;
}

} // namespace detail
} // namespace gpgpu
} // namespace cgv
