#pragma once

#include "algorithm.h"
#include "device_buffer_iterator.h"
#include "uniform_buffer.h"
#include "storage_buffer.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {
namespace detail {

class CGV_API write_if : public algorithm {
public:
	write_if(const std::string& name, bool write_indices);

	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& unary_predicate);
	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const argument_definitions& arguments, const std::string& unary_predicate);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count, const argument_bindings& arguments = {});
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, const argument_bindings& arguments = {});
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, device_buffer_iterator output_count, const argument_bindings& arguments = {});

	bool read_count(cgv::render::context& ctx, size_t& out);

private:
	struct uniform_data {
		uint32_t input_begin = 0;
		uint32_t input_end = 0;
		uint32_t output_begin = 0;
		uint32_t output_count_begin = 0;
		uint32_t num_block_sums = 0; // the number of blocksums
		uint32_t last_block_sum_idx = 0; // the index of the last valid block sum
	};

	void resize(cgv::render::context& ctx, uint32_t size);

	bool _write_indices = false;

	const uint32_t _group_size = 64;
	const uint32_t _block_size = 4 * _group_size;

	uint32_t _last_size = 0;
	uint32_t _num_groups = 0;
	uint32_t _num_block_sums = 0;
	uint32_t _last_block_sum_idx = 0;

	uniform_buffer<uniform_data> _uniform_buffer;

	compute_kernel _vote_kernel;
	compute_kernel _scan_local_kernel;
	compute_kernel _scan_global_kernel;
	compute_kernel _scatter_kernel;

	storage_buffer _votes_buffer;
	storage_buffer _prefix_sums_buffer;
	storage_buffer _block_sums_buffer;
	storage_buffer _last_sum_buffer;
};

} // namespace detail
} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
