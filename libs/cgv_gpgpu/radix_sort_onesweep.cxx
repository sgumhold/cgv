#include "radix_sort_onesweep.h"

#include <cgv/math/integer.h>

#include "double_buffer_wrapper.h"

namespace cgv {
namespace gpgpu {

radix_sort_onesweep::radix_sort_onesweep() : radix_sort("radix_sort_onesweep", 256) {
	_radix = 256;
	//register_kernel(_init_kernel, "gpgpu_radix_sort_onesweep_init");
	//register_kernel(_global_hist_kernel, "gpgpu_radix_sort_onesweep_global_histogram");
	//register_kernel(_scan_kernel, "gpgpu_radix_sort_onesweep_scan");
	//register_kernel(_digit_bin_pass_kernel, "gpgpu_radix_sort_onesweep_digit_bin_pass");
}

bool radix_sort_onesweep::v_init(cgv::render::context& ctx, cgv::render::shader_compile_options& config) {
	std::vector<compute_kernel_info> kernels = {
		{ &_init_kernel, "gpgpu_radix_sort_onesweep_init" },
		{ &_global_hist_kernel, "gpgpu_radix_sort_onesweep_global_histogram" },
		{ &_scan_kernel, "gpgpu_radix_sort_onesweep_scan" },
		{ &_digit_bin_pass_kernel, "gpgpu_radix_sort_onesweep_digit_bin_pass" }
	};

	if(init_kernels(ctx, kernels, config)) {
		// Get the maximum number of work groups that can be dispatched in dimension 0.
		_max_dispatch_dimension = ctx.get_device_capabilities().max_compute_work_group_count[0];

		// TODO: Set dynamically based on GPU specs.
		_partition_size = 3840;

		return v_resize(ctx);
	}

	return false;
}

void radix_sort_onesweep::destruct(const cgv::render::context& ctx) {
	_keys_out_buffer.destruct(ctx);
	_values_out_buffer.destruct(ctx);
	_pass_hist_buffer.destruct(ctx);
	_global_hist_buffer.destruct(ctx);
	_index_buffer.destruct(ctx);
	
	algorithm::destruct(ctx);
}

bool radix_sort_onesweep::v_resize(cgv::render::context& ctx) {
	_num_partitions = cgv::math::div_round_up(_num_keys, _partition_size);
	_num_global_histogram_partitions = cgv::math::div_round_up(_num_keys, _global_histogram_partition_size);

	_keys_out_buffer.create_or_resize<uint32_t>(ctx, _num_keys);

	if(!_value_type.is_void())
		_values_out_buffer.create_or_resize<uint32_t>(ctx, _num_keys);

	_global_hist_buffer.create_or_resize<uint32_t>(ctx, _radix * _radix_passes);
	_index_buffer.create_or_resize<uint32_t>(ctx, _radix_passes);
	_pass_hist_buffer.create_or_resize<uint32_t>(ctx, _radix * _radix_passes * _num_partitions);

	_init_kernel.enable(ctx);
	_init_kernel.set_argument(ctx, "u_thread_blocks", _num_partitions);
	_init_kernel.disable(ctx);

	_global_hist_kernel.enable(ctx);
	_global_hist_kernel.set_argument(ctx, "u_num_keys", _num_keys);
	_global_hist_kernel.set_argument(ctx, "u_thread_blocks", _num_global_histogram_partitions);
	_global_hist_kernel.disable(ctx);

	_scan_kernel.enable(ctx);
	_scan_kernel.set_argument(ctx, "u_thread_blocks", _num_partitions);
	_scan_kernel.disable(ctx);

	_digit_bin_pass_kernel.enable(ctx);
	_digit_bin_pass_kernel.set_argument(ctx, "u_num_keys", _num_keys);
	_digit_bin_pass_kernel.set_argument(ctx, "u_thread_blocks", _num_partitions);
	_digit_bin_pass_kernel.disable(ctx);

	return true;
}

void radix_sort_onesweep::v_dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* keys_buffer, const cgv::render::vertex_buffer* values_buffer) {
	vertex_double_buffer_wrapper keys(keys_buffer, &_keys_out_buffer);
	keys.binding_type_override = cgv::render::VertexBufferType::VBT_STORAGE;
	vertex_double_buffer_wrapper values(values_buffer, &_values_out_buffer);
	values.binding_type_override = cgv::render::VertexBufferType::VBT_STORAGE;

	keys.bind_all(ctx, 0, 1);
	if(!_value_type.is_void())
		values.bind_all(ctx, 2, 3);

	_global_hist_buffer.bind(ctx, 4);
	_pass_hist_buffer.bind(ctx, 5);
	_index_buffer.bind(ctx, 6);

	// Initialize auxiliary memory.
	_init_kernel.enable(ctx);
	dispatch_compute(256, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	_init_kernel.disable(ctx);

	// Perform global histogram pass.
	{
		const uint32_t k_isNotPartialBitFlag = 0;
		const uint32_t k_isPartialBitFlag = 1;

		_global_hist_kernel.enable(ctx);
		
		const uint32_t full_blocks = _num_global_histogram_partitions / _max_dispatch_dimension;
		const uint32_t partial_blocks = _num_global_histogram_partitions - full_blocks * _max_dispatch_dimension;

		if(full_blocks > 0) {
			_global_hist_kernel.set_argument(ctx, "u_is_partial", k_isNotPartialBitFlag);
			dispatch_compute(_max_dispatch_dimension, full_blocks, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}

		if(partial_blocks > 0) {
			_global_hist_kernel.set_argument(ctx, "u_is_partial", (full_blocks << 1) | k_isPartialBitFlag);
			dispatch_compute(partial_blocks, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}

		_global_hist_kernel.disable(ctx);
	}

	// Perform global scan pass.
	_scan_kernel.enable(ctx);
	dispatch_compute(_radix_passes, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	_scan_kernel.disable(ctx);

	// Perform digit binning passes.
	{
		_digit_bin_pass_kernel.enable(ctx);
		
		const uint32_t full_blocks = _num_partitions / _max_dispatch_dimension;
		const uint32_t partial_blocks = _num_partitions - full_blocks * _max_dispatch_dimension;

		// TODO: Support keys smaller than 32-bit?
		for(uint32_t i = 0; i < 32; i += 8) {
			keys.bind_all(ctx, 0, 1);
			if(!_value_type.is_void())
				values.bind_all(ctx, 2, 3);

			_digit_bin_pass_kernel.set_argument(ctx, "u_radix_shift", i);

			// Setting the partition flag here is unnecessary, because
			// we atomically assign partition tiles.
			if(full_blocks > 0) {
				dispatch_compute(_max_dispatch_dimension, full_blocks, 1);

				// To be absolutely safe, add a barrier here on the pass histogram
				// as work groups in the second dispatch are dependent on the first dispatch.
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			}

			if(partial_blocks > 0) {
				dispatch_compute(partial_blocks, 1, 1);
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			}

			keys.swap();
			values.swap();
		}

		_digit_bin_pass_kernel.disable(ctx);
	}

	keys.unbind_all(ctx, 0, 1);
	if(!_value_type.is_void()) {
		values.unbind_all(ctx, 2, 3);
	}
	_pass_hist_buffer.unbind(ctx, 4);
	_global_hist_buffer.unbind(ctx, 5);
	_index_buffer.unbind(ctx, 6);
}

} // namespace gpgpu
} // namespace cgv
