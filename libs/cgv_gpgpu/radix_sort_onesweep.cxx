#include "radix_sort_onesweep.h"

#include <cgv/utils/scan.h>

#include "utils.h"

namespace cgv {
namespace gpgpu {

radix_sort_onesweep::radix_sort_onesweep() : sort("radix_sort_onesweep") {
	register_kernel(_init_kernel, "gpgpu_radix_sort_onesweep_init");
	register_kernel(_global_hist_kernel, "gpgpu_radix_sort_onesweep_global_histogram");
	register_kernel(_scan_kernel, "gpgpu_radix_sort_onesweep_scan");
	register_kernel(_digit_bin_pass_kernel, "gpgpu_radix_sort_onesweep_digit_bin_pass");
}

bool radix_sort_onesweep::v_init(cgv::render::context& ctx) {
	cgv::render::shader_compile_options config;

	config.defines["KEY_TYPE"] = "KEY_" + cgv::utils::to_upper(to_string(_key_type.type()));
	config.defines["SORT_ASCENDING"] = _order == Order::kAscending ? "1" : "0";
	config.defines["SORT_PAIRS"] = _value_type.is_void() ? "0" : "1";
	config.defines["PAYLOAD_TYPE"] = "PAYLOAD_" + cgv::utils::to_upper(to_string(_value_type.type()));




	//cgv::render::shader_code::set_define(defines, "VALUE_TYPEDEF", value_typedef, "uint");

	//res = res && cgv::render::shader_library::load(ctx, init_prog, "radix_sort_onesweep_init", defines, true, where);
	//res = res && cgv::render::shader_library::load(ctx, global_hist_prog, "radix_sort_onesweep_global_histogram", defines, true, where);
	//res = res && cgv::render::shader_library::load(ctx, scan_prog, "radix_sort_onesweep_scan", defines, true, where);
	//res = res && cgv::render::shader_library::load(ctx, digit_bin_pass_prog, "radix_sort_onesweep_digit_bin_pass", defines, true, where);

	//global_hist_uniforms = get_program_uniforms(ctx, global_hist_prog);
	//scan_uniforms = get_program_uniforms(ctx, scan_prog);
	//digit_bin_pass_uniforms = get_program_uniforms(ctx, digit_bin_pass_prog);

	if(init_kernels(ctx, config)) {
		// TODO: Set dynamically based on GPU specs.
		_partition_size = 3840;

		_num_partitions = div_round_up(_num_keys, _partition_size);
		_num_global_histogram_partitions = div_round_up(_num_keys, _global_histogram_partition_size);

		ensure_buffer(ctx, _keys_out_buffer, _num_keys * sizeof(uint32_t));

		if(!_value_type.is_void())
			ensure_buffer(ctx, _values_out_buffer, _num_keys * sizeof(uint32_t));

		ensure_buffer(ctx, _global_hist_buffer, _radix * _radix_passes * sizeof(uint32_t));
		ensure_buffer(ctx, _index_buffer, _radix_passes * sizeof(uint32_t));
		ensure_buffer(ctx, _pass_hist_buffer, _radix * _radix_passes * _num_partitions * sizeof(uint32_t));

		_init_kernel.enable(ctx);
		_init_kernel.set_argument(ctx, "u_thread_blocks", _num_partitions);
		_init_kernel.disable(ctx);

		_global_hist_kernel.enable(ctx);
		_global_hist_kernel.set_argument(ctx, "u_num_keys", _num_keys);
		_global_hist_kernel.disable(ctx);

		_digit_bin_pass_kernel.enable(ctx);
		_digit_bin_pass_kernel.set_argument(ctx, "u_num_keys", _num_keys);
		_digit_bin_pass_kernel.disable(ctx);

		return true;
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

void radix_sort_onesweep::v_dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* keys_buffer, const cgv::render::vertex_buffer* values_buffer) {
	cgv::gpgpu::double_buffer_wrapper<const cgv::render::vertex_buffer> keys(keys_buffer, &_keys_out_buffer);
	cgv::gpgpu::double_buffer_wrapper<const cgv::render::vertex_buffer> values(values_buffer, &_values_out_buffer);

	keys.first()->bind(ctx, 0);
	keys.second()->bind(ctx, 1);
	if(!_value_type.is_void()) {
		values.first()->bind(ctx, 2);
		values.second()->bind(ctx, 3);
	}

	_global_hist_buffer.bind(ctx, 4);
	_pass_hist_buffer.bind(ctx, 5);
	_index_buffer.bind(ctx, 6);

	_init_kernel.enable(ctx);
	dispatch_compute(256, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	_init_kernel.disable(ctx);

	{
		uint32_t thread_blocks = _num_global_histogram_partitions;
		const uint32_t k_isNotPartialBitFlag = 0;
		const uint32_t k_isPartialBitFlag = 1;

		const uint32_t k_maxDim = 65535;
		const uint32_t fullBlocks = thread_blocks / k_maxDim;
		const uint32_t partialBlocks = thread_blocks - fullBlocks * k_maxDim;

		// TODO: handle fullBlocks > 0

		_global_hist_kernel.enable(ctx);
		_global_hist_kernel.set_argument(ctx, "u_is_partial", (fullBlocks << 1) | k_isPartialBitFlag);
		_global_hist_kernel.set_argument(ctx, "u_thread_blocks", thread_blocks);

		dispatch_compute(partialBlocks, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		_global_hist_kernel.disable(ctx);
	}

	_scan_kernel.enable(ctx);
	_scan_kernel.set_argument(ctx, "u_thread_blocks", _num_partitions);
	dispatch_compute(_radix_passes, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	_scan_kernel.disable(ctx);

	// TODO: handle fullBlocks > 0
	{
		uint32_t thread_blocks = _num_partitions;

		const uint32_t k_maxDim = 65535;
		const uint32_t fullBlocks = thread_blocks / k_maxDim;
		const uint32_t partialBlocks = thread_blocks - fullBlocks * k_maxDim;

		_digit_bin_pass_kernel.enable(ctx);
		_digit_bin_pass_kernel.set_argument(ctx, "u_thread_blocks", thread_blocks);

		// TODO: Support keys smaller than 32-bit?
		for(uint32_t i = 0; i < 32; i += 8) {
			keys.first()->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
			keys.second()->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
			if(!_value_type.is_void()) {
				values.first()->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 2);
				values.second()->bind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 3);
			}
			_digit_bin_pass_kernel.set_argument(ctx, "u_radix_shift", i);
			dispatch_compute(partialBlocks, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			keys.swap();
			values.swap();
		}

		_digit_bin_pass_kernel.disable(ctx);
	}

	keys.first()->unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 0);
	keys.second()->unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 1);
	if(!_value_type.is_void()) {
		values.first()->unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 2);
		values.second()->unbind(ctx, cgv::render::VertexBufferType::VBT_STORAGE, 3);
	}
	_pass_hist_buffer.unbind(ctx, 4);
	_global_hist_buffer.unbind(ctx, 5);
	_index_buffer.unbind(ctx, 6);
}

} // namespace gpgpu
} // namespace cgv
