#include "copy_if.h"

#include <cgv/math/integer.h>

namespace cgv {
namespace gpgpu {

copy_if::copy_if() : algorithm("copy_if") {}

bool copy_if::init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& unary_predicate) {
	return init(ctx, value_type, {}, unary_predicate);
}

bool copy_if::init(cgv::render::context& ctx, const sl::data_type& value_type, const argument_definitions& arguments, const std::string& unary_predicate) {
	if(!value_type.is_valid())
		return false;

	//_value_type = value_type;
	//_num_values = static_cast<uint32_t>(count);

	algorithm_create_info info;
	info.arguments = &arguments;
	info.types.push_back(value_type);
	info.typedefs.push_back({ "value_type", value_type });
	info.default_buffer_count = 5;
	info.options.define_snippet("predicate", unary_predicate);

	std::vector<compute_kernel_info> kernels = {
		{ &_vote_kernel, "gpgpu_copy_if_vote" },
		{ &_scan_local_kernel, "gpgpu_copy_if_scan_local" },
		{ &_scan_global_kernel, "gpgpu_copy_if_scan_global" },
		{ &_scatter_kernel, "gpgpu_copy_if_scatter" }
	};

	return algorithm::init(ctx, info, kernels);

	/*
	if(algorithm::init(ctx, info, kernels))
		return resize(ctx);

	return false;
	*/
}

void copy_if::destruct(const cgv::render::context& ctx) {
	_vote_kernel.destruct(ctx);
	_scan_local_kernel.destruct(ctx);
	_scan_global_kernel.destruct(ctx);
	_scatter_kernel.destruct(ctx);
	_votes_buffer.destruct(ctx);
	_prefix_sums_buffer.destruct(ctx);
	_block_sums_buffer.destruct(ctx);
	algorithm::destruct(ctx);
}

void copy_if::resize(cgv::render::context& ctx, uint32_t size) {
	// Todo: Clean up num groups (currently unused) and num scan groups.

	// Pad number of values to the next multiple of blocksize.
	uint32_t num_values_padded = cgv::math::next_multiple_k_greater_than_n(_block_size, size);
	_num_groups = cgv::math::div_round_up(num_values_padded, _group_size);
	_num_scan_groups = cgv::math::div_round_up(num_values_padded, _block_size);
	_num_block_sums = cgv::math::next_power_of_two(_num_scan_groups);
	uint32_t block_sum_offset_shift = static_cast<uint32_t>(std::log2(_block_size));
	uint32_t num_vote_ballots = cgv::math::div_round_up(num_values_padded, 32u);

	_votes_buffer.create_or_resize<uint32_t>(ctx, num_vote_ballots);
	_prefix_sums_buffer.create_or_resize<uint32_t>(ctx, num_values_padded);
	_block_sums_buffer.create_or_resize<uint32_t>(ctx, 4ull * _num_block_sums);

	uint32_t last_block_sum_idx = (num_values_padded >> block_sum_offset_shift) - 1;

	// Todo: Is num values padded actually needed? It should work without.
	// Todo: Remove uniforms that are not used by individual kernels.

	_vote_kernel.enable(ctx);
	_vote_kernel.set_argument(ctx, "u_num_values", size);
	_vote_kernel.set_argument(ctx, "u_num_values_padded", num_values_padded);
	_vote_kernel.disable(ctx);

	_scan_local_kernel.enable(ctx);
	_scan_local_kernel.set_argument(ctx, "u_num_scan_groups", _num_scan_groups);
	_scan_local_kernel.disable(ctx);

	_scan_global_kernel.enable(ctx);
	_scan_global_kernel.set_argument(ctx, "u_num_block_sums", _num_block_sums);
	_scan_global_kernel.set_argument(ctx, "u_last_block_sum_idx", last_block_sum_idx);
	_scan_global_kernel.disable(ctx);

	_scatter_kernel.enable(ctx);
	_scatter_kernel.set_argument(ctx, "u_num_values", size);
	_scatter_kernel.disable(ctx);
}

bool copy_if::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count, const argument_bindings& arguments) {
	return dispatch(ctx, begin(input_buffer), begin(input_buffer) + count, begin(output_buffer), arguments);
}

bool copy_if::dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, const argument_bindings& arguments) {
	if(!is_valid_range(input_first, input_last))
		return false;

	if(compatible(input_first, output_first))
		return false;
	

	//const size_t max_count = 0xFFFFFFFF;
	//if(count == 0 || count > max_count)
	//	return false;

	// Todo: Decide on exact vote kernel implementation based on available group count.
	auto temp = ctx.get_device_capabilities().max_compute_work_group_count.x();


	uint32_t count = static_cast<uint32_t>(cgv::gpgpu::distance(input_first, input_last));
	resize(ctx, count);



	// Todo: Currently, the input range size must exactly match _num_values!
	// Todo: Implement actual support for iterators.

	input_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
	_votes_buffer.bind(ctx, 2);
	
	_vote_kernel.enable(ctx);
	_vote_kernel.set_argument<uint32_t>(ctx, "u_input_begin", input_first.index());
	_vote_kernel.set_argument<uint32_t>(ctx, "u_input_end", input_last.index());
	_vote_kernel.set_argument<uint32_t>(ctx, "u_output_begin", output_first.index());
	_vote_kernel.set_arguments(ctx, arguments);
	bind_buffer_like_arguments(ctx, arguments);

	dispatch_compute(_num_scan_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	
	unbind_buffer_like_arguments(ctx, arguments);
	_vote_kernel.disable(ctx);
	
	_prefix_sums_buffer.bind(ctx, 3);
	_block_sums_buffer.bind(ctx, 4);

	_scan_local_kernel.enable(ctx);
	dispatch_compute(_num_scan_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	_scan_local_kernel.disable(ctx);

	_scan_global_kernel.enable(ctx);
	dispatch_compute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	_scan_global_kernel.disable(ctx);

	_scatter_kernel.enable(ctx);
	dispatch_compute(_num_scan_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	_scatter_kernel.disable(ctx);

	input_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
	_votes_buffer.unbind(ctx, 2);
	_prefix_sums_buffer.unbind(ctx, 3);
	_block_sums_buffer.unbind(ctx, 4);

	return true;
}









copy_if_atomic::copy_if_atomic() : algorithm("copy_if_atomic") {}

bool copy_if_atomic::init(cgv::render::context& ctx, const sl::data_type& value_type, size_t count, const std::string& unary_predicate) {
	return init(ctx, value_type, count, {}, unary_predicate);
}

bool copy_if_atomic::init(cgv::render::context& ctx, const sl::data_type& value_type, size_t count, const argument_definitions& arguments, const std::string& unary_predicate) {
	if(!value_type.is_valid())
		return false;

	const size_t max_count = 0xFFFFFFFF;
	if(count == 0 || count > max_count)
		return false;

	_value_type = value_type;
	_num_values = static_cast<uint32_t>(count);

	algorithm_create_info info;
	info.arguments = &arguments;
	info.types.push_back(value_type);
	info.typedefs.push_back({ "value_type", value_type });
	info.default_buffer_count = 3;
	info.options.define_snippet("predicate", unary_predicate);

	if(algorithm::init(ctx, info, { { &_kernel, "gpgpu_copy_if_atomic" }, }))
		return resize(ctx);

	return false;
}

void copy_if_atomic::destruct(const cgv::render::context& ctx) {
	_kernel.destruct(ctx);
	_atomic_counter_buffer.destruct(ctx);
	algorithm::destruct(ctx);
}

bool copy_if_atomic::resize(cgv::render::context& ctx) {
	// Pad number of values to the next multiple of blocksize.
	_num_groups = cgv::math::div_round_up(_num_values, _group_size);
	
	_atomic_counter_buffer.create_or_resize<int32_t>(ctx, 1);
	
	// Todo: Is num values padded actually needed? It should work without.
	// Todo: Remove uniforms that are not used by individual kernels.

	_kernel.enable(ctx);
	_kernel.set_argument(ctx, "u_num_values", _num_values);
	_kernel.disable(ctx);

	return true;
}

bool copy_if_atomic::dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count, const argument_bindings& arguments) {
	return dispatch(ctx, begin(input_buffer), begin(input_buffer) + count, begin(output_buffer), arguments);
}

bool copy_if_atomic::dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, const argument_bindings& arguments) {
	if(!is_valid_range(input_first, input_last))
		return false;

	if(compatible(input_first, output_first))
		return false;

	// Todo: Reset atomic_counter_buffer to zero.

	_atomic_counter_buffer.ctx_ptr = &ctx;
	GLuint id = 0;
	_atomic_counter_buffer.put_id(id);
	int32_t zero = 0;
	glNamedBufferSubData(id, 0, sizeof(int32_t), &zero);

	
	// Todo: Currently, the input range size must exactly match _num_values!
	// Todo: Implement actual support for iterators.

	input_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
	_atomic_counter_buffer.bind(ctx, 2);

	_kernel.enable(ctx);
	_kernel.set_argument<uint32_t>(ctx, "u_input_begin", input_first.index());
	_kernel.set_argument<uint32_t>(ctx, "u_input_end", input_last.index());
	_kernel.set_argument<uint32_t>(ctx, "u_output_begin", output_first.index());
	_kernel.set_arguments(ctx, arguments);
	bind_buffer_like_arguments(ctx, arguments);

	dispatch_compute(_num_groups, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	unbind_buffer_like_arguments(ctx, arguments);
	_kernel.disable(ctx);

	input_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	output_first.buffer().unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
	_atomic_counter_buffer.unbind(ctx, 2);

	return true;
}

} // namespace gpgpu
} // namespace cgv
