#pragma once

#include "algorithm.h"
#include "device_buffer_iterator.h"
#include "storage_buffer.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

//enum class CopyMode {
//	kCopy,
//	kIndex
//};

/*
namespace detail {

/// GPU compute shader implementation for copying values based on a boolean predicate.
class CGV_API copy_if_base : public algorithm {
public:
	copy_if_base();

	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& unary_predicate);
	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const argument_definitions& arguments, const std::string& unary_predicate);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count, const argument_bindings& arguments = {});
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, const argument_bindings& arguments = {});

	bool read_count(cgv::render::context& ctx, size_t& out) {
		uint32_t count = 0;
		//if(_prefix_sums_buffer.copy(ctx, 0, &count, 1)) {
		//	out = count;
		//	return true;
		//}
		return false;
	}

private:
};

} // detail
*/

//enum class MemoryPolicy {
//	kUnrestricted,
//	kConservative
//};

/// GPU compute shader implementation for copying values based on a boolean predicate.
class CGV_API copy_if : public algorithm {
public:
	copy_if();

	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& unary_predicate);
	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const argument_definitions& arguments, const std::string& unary_predicate);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count, const argument_bindings& arguments = {});
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, const argument_bindings& arguments = {});

	bool read_count(cgv::render::context& ctx, size_t& out) {
		uint32_t count = 0;
		if(_prefix_sums_buffer.copy(ctx, 0, &count, 1)) {
			out = count;
			return true;
		}
		return false;
	}

private:
	void resize(cgv::render::context& ctx, uint32_t size);

	//CopyMode mode = CopyMode::kCopy;
	
	const uint32_t _group_size = 64;
	const uint32_t _block_size = 4 * _group_size;

	uint32_t _last_size = 0;
	uint32_t _num_groups = 0;
	uint32_t _num_block_sums = 0;
	uint32_t _last_block_sum_idx = 0;

	compute_kernel _vote_kernel;
	compute_kernel _scan_local_kernel;
	compute_kernel _scan_global_kernel;
	compute_kernel _scatter_kernel;

	storage_buffer _votes_buffer;
	storage_buffer _prefix_sums_buffer;
	storage_buffer _block_sums_buffer;
};

class CGV_API copy_if_atomic : public algorithm {
public:
	copy_if_atomic();

	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const std::string& unary_predicate);
	bool init(cgv::render::context& ctx, const sl::data_type& value_type, const argument_definitions& arguments, const std::string& unary_predicate);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& input_buffer, const cgv::render::vertex_buffer& output_buffer, size_t count, const argument_bindings& arguments = {});
	bool dispatch(cgv::render::context& ctx, device_buffer_iterator input_first, device_buffer_iterator input_last, device_buffer_iterator output_first, const argument_bindings& arguments = {});

	bool read_count(cgv::render::context& ctx, size_t& out) {
		uint32_t count = 0;
		if(_atomic_counter_buffer.copy(ctx, 0, &count, 1)) {
			out = count;
			return true;
		}
		return false;
	}

private:
	compute_kernel _kernel;
	
	storage_buffer _atomic_counter_buffer;
};

}
}

#include <cgv/config/lib_end.h>
