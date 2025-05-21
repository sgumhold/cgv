#pragma once

#include "radix_sort.h"
#include "storage_buffer.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

class CGV_API radix_sort_4x : public radix_sort {
public:
	radix_sort_4x();

	void destruct(const cgv::render::context& ctx) override;

private:
	bool v_init(cgv::render::context& ctx, cgv::render::shader_compile_options& config) override;

	void v_dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer* keys_buffer, const cgv::render::vertex_buffer* values_buffer) override;

	const uint32_t _group_size = 64;
	const uint32_t _block_size = 4 * _group_size;

	uint32_t _num_block_sums = 0;
	uint32_t _num_groups = 0;
	uint32_t _num_scan_groups = 0;

	compute_kernel _scan_local_kernel;
	compute_kernel _scan_global_kernel;
	compute_kernel _scatter_kernel;

	/// GPU buffers
	storage_buffer _keys_out_buffer;
	storage_buffer _values_out_buffer;
	storage_buffer _prefix_sums_buffer;
	storage_buffer _block_sums_buffer;
	storage_buffer _last_sum_buffer;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
