#include "radix_sort_4x.h"

#include <cgv/math/integer.h>

#include "double_buffer_wrapper.h"

namespace cgv {
namespace gpgpu {

radix_sort_4x::radix_sort_4x() : radix_sort("radix_sort_4x", 4) {}

bool radix_sort_4x::v_init(cgv::render::context& ctx, cgv::render::shader_compile_options& config) {
	std::vector<compute_kernel_info> kernels = {
		{ &_scan_local_kernel, "gpgpu_radix_sort_4x_scan_local" },
		{ &_scan_global_kernel, "gpgpu_radix_sort_4x_scan_global" },
		{ &_scatter_kernel, "gpgpu_radix_sort_4x_scatter" }
	};

	algorithm_create_info info;
	info.options = config;

	if(algorithm::init(ctx, info, kernels))
		return v_resize(ctx);

	return false;
}

void radix_sort_4x::destruct(const cgv::render::context& ctx) {
	_scan_local_kernel.destruct(ctx);
	_scan_global_kernel.destruct(ctx);
	_scatter_kernel.destruct(ctx);
	_keys_out_buffer.destruct(ctx);
	_values_out_buffer.destruct(ctx);
	_prefix_sums_buffer.destruct(ctx);
	_block_sums_buffer.destruct(ctx);
	_last_sum_buffer.destruct(ctx);
	algorithm::destruct(ctx);
}

bool radix_sort_4x::v_resize(cgv::render::context& ctx) {
	// Pad number of keys to the next multiple of blocksize.
	uint32_t num_keys_padded = cgv::math::next_multiple_k_greater_than_n(_block_size, _num_keys);
	_num_groups = cgv::math::div_round_up(num_keys_padded, _group_size);
	_num_scan_groups = cgv::math::div_round_up(num_keys_padded, _block_size);
	_num_block_sums = cgv::math::next_power_of_two(_num_scan_groups);
	uint32_t block_sum_offset_shift = static_cast<uint32_t>(std::log2(_block_size));

	// The keys and values output buffer do not need to be padded because no writes will happen beyond the actual key count.
	_keys_out_buffer.create_or_resize<uint32_t>(ctx, _num_keys);

	if(!_value_type.is_void()) {
		// TODO: Remove temporary and use actual component count.
		uint32_t temp_value_component_count = 1;
		_values_out_buffer.create_or_resize<uint32_t>(ctx, static_cast<size_t>(temp_value_component_count) * _num_keys);
	}

	_prefix_sums_buffer.create_or_resize<uint32_t>(ctx, num_keys_padded);
	_block_sums_buffer.create_or_resize<uint32_t>(ctx, 4ull * _num_block_sums);
	_last_sum_buffer.create_or_resize<uint32_t>(ctx, 4);

	uint32_t last_block_sum_idx = (num_keys_padded >> block_sum_offset_shift) - 1;

	_scan_local_kernel.enable(ctx);
	_scan_local_kernel.set_argument(ctx, "u_num_keys", _num_keys);
	_scan_local_kernel.set_argument(ctx, "u_num_scan_groups", _num_scan_groups);
	_scan_local_kernel.set_argument(ctx, "u_num_block_sums", _num_block_sums);
	_scan_local_kernel.disable(ctx);

	_scan_global_kernel.enable(ctx);
	_scan_global_kernel.set_argument(ctx, "u_num_block_sums", _num_block_sums);
	_scan_global_kernel.disable(ctx);

	_scatter_kernel.enable(ctx);
	_scatter_kernel.set_argument(ctx, "u_num_keys", _num_keys);
	_scatter_kernel.set_argument(ctx, "u_num_block_sums", _num_block_sums);
	_scatter_kernel.set_argument(ctx, "u_last_block_sum_idx", last_block_sum_idx);
	_scatter_kernel.disable(ctx);

	return true;
}

void radix_sort_4x::v_dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* keys_buffer, const cgv::render::vertex_buffer* values_buffer) {
	vertex_double_buffer_wrapper keys(keys_buffer, &_keys_out_buffer);
	keys.binding_type_override = cgv::render::VertexBufferType::VBT_STORAGE;
	vertex_double_buffer_wrapper values(values_buffer, &_values_out_buffer);
	values.binding_type_override = cgv::render::VertexBufferType::VBT_STORAGE;

	_prefix_sums_buffer.bind(ctx, 4);
	_block_sums_buffer.bind(ctx, 5);
	_last_sum_buffer.bind(ctx, 6);

	constexpr GLbitfield barrier = GL_SHADER_STORAGE_BARRIER_BIT;

	for(uint32_t i = 0; i < 32; i += 2) {
		keys.bind_all(ctx, 0, 1);
		if(!_value_type.is_void())
			values.bind_all(ctx, 2, 3);

		_scan_local_kernel.enable(ctx);
		_scan_local_kernel.set_argument(ctx, "u_radix_shift", i);
		dispatch_compute(_num_scan_groups, 1, 1);
		glMemoryBarrier(barrier);
		_scan_local_kernel.disable(ctx);

		_scan_global_kernel.enable(ctx);
		dispatch_compute(4, 1, 1);
		glMemoryBarrier(barrier);
		_scan_global_kernel.disable(ctx);

		_scatter_kernel.enable(ctx);
		_scatter_kernel.set_argument(ctx, "u_radix_shift", i);
		dispatch_compute(_num_scan_groups, 1, 1);
		glMemoryBarrier(barrier);
		_scatter_kernel.disable(ctx);

		keys.swap();
		values.swap();
	}

	keys.unbind_all(ctx, 0, 1);
	if(!_value_type.is_void())
		values.unbind_all(ctx, 2, 3);
	_prefix_sums_buffer.unbind(ctx, 4);
	_block_sums_buffer.unbind(ctx, 5);
	_last_sum_buffer.unbind(ctx, 6);
}

} // namespace gpgpu
} // namespace cgv
