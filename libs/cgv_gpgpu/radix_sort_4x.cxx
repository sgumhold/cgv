#include "radix_sort_4x.h"

//#include <cgv/utils/scan.h>

#include "utils.h"

namespace cgv {
namespace gpgpu {

radix_sort_4x::radix_sort_4x() : radix_sort("radix_sort_4x", 4) {
	register_kernel(_scan_local_kernel, "gpgpu_radix_sort_4x_scan_local");
	register_kernel(_scan_global_kernel, "gpgpu_radix_sort_4x_scan_global");
	register_kernel(_scatter_kernel, "gpgpu_radix_sort_4x_scatter");
}

bool radix_sort_4x::v_init(cgv::render::context& ctx, cgv::render::shader_compile_options& config) {
	if(init_kernels(ctx, config)) {
		// Pad numer of keys to the next multiple of blocksize.
		uint32_t num_keys_padded = next_multiple_greater_than(_num_keys, _block_size);

		_num_groups = div_round_up(num_keys_padded, _group_size);
		_num_scan_groups = div_round_up(num_keys_padded, _block_size);
		_num_block_sums = next_power_of_two(_num_scan_groups);
		uint32_t block_sum_offset_shift = static_cast<uint32_t>(std::log2(_block_size));

		// TODO: Can we ignore padding in both "out" buffers?
		ensure_buffer(ctx, _keys_out_buffer, num_keys_padded * sizeof(uint32_t));

		if(!_value_type.is_void()) {
			// TODO: Remove temporary and use actual component count.
			uint32_t temp_value_component_count = 1;
			ensure_buffer(ctx, _values_out_buffer, static_cast<size_t>(temp_value_component_count * num_keys_padded * sizeof(uint32_t)));
		}

		ensure_buffer(ctx, _prefix_sums_buffer, num_keys_padded * sizeof(uint32_t) / 4);
		ensure_buffer(ctx, _block_sums_buffer, 4 * _num_block_sums * sizeof(uint32_t));
		ensure_buffer(ctx, _last_sum_buffer, 4 * sizeof(uint32_t));

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

	return false;
}

void radix_sort_4x::destruct(const cgv::render::context& ctx) {
	_keys_out_buffer.destruct(ctx);
	_values_out_buffer.destruct(ctx);
	_prefix_sums_buffer.destruct(ctx);
	_block_sums_buffer.destruct(ctx);
	_last_sum_buffer.destruct(ctx);
	
	algorithm::destruct(ctx);
}

void radix_sort_4x::v_dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* keys_buffer, const cgv::render::vertex_buffer* values_buffer) {
	cgv::gpgpu::double_buffer_wrapper<const cgv::render::vertex_buffer> keys(keys_buffer, &_keys_out_buffer);
	cgv::gpgpu::double_buffer_wrapper<const cgv::render::vertex_buffer> values(values_buffer, &_values_out_buffer);

	_prefix_sums_buffer.bind(ctx, 4);
	_block_sums_buffer.bind(ctx, 5);
	_last_sum_buffer.bind(ctx, 6);

	// TODO: Check scatter address if padding > 0!

	constexpr GLbitfield barrier = GL_SHADER_STORAGE_BARRIER_BIT;

	for(uint32_t i = 0; i < 32; i += 2) {
		keys.first()->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
		keys.second()->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
		if(!_value_type.is_void()) {
			values.first()->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 2);
			values.second()->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 3);
		}

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

	keys.first()->unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	keys.second()->unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
	if(!_value_type.is_void()) {
		values.first()->unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 2);
		values.second()->unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 3);
	}
	_prefix_sums_buffer.unbind(ctx, 4);
	_block_sums_buffer.unbind(ctx, 5);
	_last_sum_buffer.unbind(ctx, 6);
}

} // namespace gpgpu
} // namespace cgv
