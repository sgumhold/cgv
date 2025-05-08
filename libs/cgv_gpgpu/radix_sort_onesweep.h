#pragma once

#include "sort.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

class CGV_API radix_sort_onesweep : public sort {
public:
	radix_sort_onesweep();

	void destruct(const cgv::render::context& ctx);

private:
	bool v_init(cgv::render::context& ctx) override;

	void v_dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* keys_buffer, const cgv::render::vertex_buffer* values_buffer) override;

	/*
	struct tuning_parameters_t {
		bool lock_waves_to_32;
		uint32_t keys_per_thread;
		uint32_t threads_per_thread_block;
		uint32_t partition_size;
		uint32_t total_shared_memory;
	};
	*/

	const uint32_t _radix = 256;
	const uint32_t _radix_passes = 4;
	const uint32_t _global_histogram_partition_size = 32768;
	const uint32_t _max_dispatch_dimension = 65535;

	uint32_t _partition_size = 0;
	uint32_t _num_partitions = 0;
	uint32_t _num_global_histogram_partitions = 0;

	compute_kernel _init_kernel;
	compute_kernel _global_hist_kernel;
	compute_kernel _scan_kernel;
	compute_kernel _digit_bin_pass_kernel;

	/// GPU buffers
	cgv::render::vertex_buffer _keys_out_buffer;
	cgv::render::vertex_buffer _values_out_buffer;
	cgv::render::vertex_buffer _pass_hist_buffer;
	cgv::render::vertex_buffer _global_hist_buffer;
	cgv::render::vertex_buffer _index_buffer;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
